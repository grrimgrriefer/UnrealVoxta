// Georgy Treshchev 2024.

#include "ImportedSoundWave.h"
#include "AudioDevice.h"
#include "Async/Async.h"
#include "Async/Future.h"
#include "Engine/Engine.h"
#include "AudioThread.h"
#include "AudioDeviceHandle.h"
#include "RAW_RuntimeCodec.h"
#include "RuntimeAudioImporterLibrary.h"

UImportedSoundWave::UImportedSoundWave(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DataGuard(MakeShared<FCriticalSection>())
	, PlaybackFinishedBroadcast(false)
	, PlayedNumOfFrames(0)
	, PCMBufferInfo(MakeShared<FPCMStruct>())
	, bStopSoundOnPlaybackFinish(true)
	, ImportedAudioFormat(ERuntimeAudioFormat::Invalid)
{
	ensure(PCMBufferInfo);

	SetImportedSampleRate(0);
	SetSampleRate(0);
	NumChannels = 0;
	Duration = 0;
	bProcedural = true;
	DecompressionType = EDecompressionType::DTYPE_Procedural;
	SoundGroup = ESoundGroup::SOUNDGROUP_Default;
	SetPrecacheState(ESoundWavePrecacheState::Done);
}

UImportedSoundWave* UImportedSoundWave::CreateImportedSoundWave()
{
	if (!IsInGameThread())
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to create a sound wave outside of the game thread"));
		return nullptr;
	}

	return NewObject<UImportedSoundWave>();
}

void UImportedSoundWave::DuplicateSoundWave(bool bUseSharedAudioBuffer, const FOnDuplicateSoundWave& Result)
{
	DuplicateSoundWave(bUseSharedAudioBuffer, FOnDuplicateSoundWaveNative::CreateWeakLambda(this, [Result] (bool bSucceeded, UImportedSoundWave* DuplicatedSoundWave)
		{
			Result.ExecuteIfBound(bSucceeded, DuplicatedSoundWave);
		}));
}

void UImportedSoundWave::DuplicateSoundWave(bool bUseSharedAudioBuffer, const FOnDuplicateSoundWaveNative& Result)
{
	if (IsInGameThread())
	{
		AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [WeakThis = MakeWeakObjectPtr(this), bUseSharedAudioBuffer, Result] ()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->DuplicateSoundWave(bUseSharedAudioBuffer, Result);
			}
			else
			{
				Result.ExecuteIfBound(false, nullptr);
			}
		});
		return;
	}

	auto ExecuteResult = [Result] (bool bSucceeded, UImportedSoundWave* DuplicatedSoundWave)
		{
			AsyncTask(ENamedThreads::GameThread, [Result, bSucceeded, DuplicatedSoundWave] ()
			{
				if (DuplicatedSoundWave)
				{
					DuplicatedSoundWave->ClearInternalFlags(EInternalObjectFlags::Async);
				}
				Result.ExecuteIfBound(bSucceeded, DuplicatedSoundWave);
			});
		};

	UImportedSoundWave* DuplicatedSoundWave = NewObject<UImportedSoundWave>(GetOuter());
	if (!DuplicatedSoundWave)
	{
		UE_LOG(AudioLog, Error, TEXT("Failed to duplicate the imported sound wave '%s'"), *GetName());
		Result.ExecuteIfBound(false, nullptr);
		return;
	}
	DuplicatedSoundWave->SetInternalFlags(EInternalObjectFlags::Async);
	FRAIScopeLock Lock(&*DataGuard);
	DuplicatedSoundWave->PCMBufferInfo = bUseSharedAudioBuffer ? PCMBufferInfo : MakeShared<FPCMStruct>(*PCMBufferInfo);
	DuplicatedSoundWave->bStopSoundOnPlaybackFinish = bStopSoundOnPlaybackFinish;
	DuplicatedSoundWave->ImportedAudioFormat = ImportedAudioFormat;
	DuplicatedSoundWave->Duration = Duration;
	DuplicatedSoundWave->SetSampleRate(GetSampleRate());
	DuplicatedSoundWave->NumChannels = NumChannels;
	if (bUseSharedAudioBuffer)
	{
		DuplicatedSoundWave->DataGuard = DataGuard;
	}
	ExecuteResult(true, DuplicatedSoundWave);
}

Audio::EAudioMixerStreamDataFormat::Type UImportedSoundWave::GetGeneratedPCMDataFormat() const
{
	return Audio::EAudioMixerStreamDataFormat::Type::Float;
}

int32 UImportedSoundWave::OnGeneratePCMAudio(TArray<uint8>& OutAudio, int32 NumSamples)
{
	TArray<float> PCMData;
	{
		FRAIScopeLock Lock(&*DataGuard);

		if (!PCMBufferInfo.IsValid())
		{
			return 0;
		}

		// Ensure there is enough number of frames. Lack of frames means audio playback has finished
		if (GetNumOfPlayedFrames_Internal() >= PCMBufferInfo->PCMNumOfFrames)
		{
			return 0;
		}

		// Getting the remaining number of samples if the required number of samples is greater than the total available number
		if (GetNumOfPlayedFrames_Internal() + (static_cast<uint32>(NumSamples) / static_cast<uint32>(NumChannels)) >= PCMBufferInfo->PCMNumOfFrames)
		{
			NumSamples = (PCMBufferInfo->PCMNumOfFrames - GetNumOfPlayedFrames_Internal()) * NumChannels;
		}

		// Retrieving a part of PCM data
		float* RetrievedPCMDataPtr = PCMBufferInfo->PCMData.GetView().GetData() + (GetNumOfPlayedFrames_Internal() * NumChannels);
		const int32 RetrievedPCMDataSize = NumSamples * sizeof(float);

		// Ensure we got a valid PCM data
		if (RetrievedPCMDataSize <= 0 || !RetrievedPCMDataPtr)
		{
			UE_LOG(AudioLog, Error, TEXT("Unable to get PCM audio from imported sound wave since the retrieved PCM data is invalid"));
			return 0;
		}

		// Filling in OutAudio array with the retrieved PCM data
		OutAudio = TArray<uint8>(reinterpret_cast<uint8*>(RetrievedPCMDataPtr), RetrievedPCMDataSize);

		// Increasing the number of frames played
		SetNumOfPlayedFrames_Internal(GetNumOfPlayedFrames_Internal() + (NumSamples / NumChannels));

		PCMData = TArray<float>(RetrievedPCMDataPtr, NumSamples);
	}

	const bool IsBound = [this] ()
		{
			FRAIScopeLock Lock(&OnGeneratePCMData_DataGuard);
			return OnGeneratePCMDataNative.IsBound() || OnGeneratePCMData.IsBound();
		}();
		
	if (IsBound)
	{
		AsyncTask(ENamedThreads::GameThread, [WeakThis = MakeWeakObjectPtr(this), PCMData = MoveTemp(PCMData)] () mutable
		{
			if (WeakThis.IsValid())
			{
				FRAIScopeLock Lock(&WeakThis->OnGeneratePCMData_DataGuard);
				if (WeakThis->OnGeneratePCMDataNative.IsBound())
				{
					WeakThis->OnGeneratePCMDataNative.Broadcast(PCMData);
				}

				if (WeakThis->OnGeneratePCMData.IsBound())
				{
					WeakThis->OnGeneratePCMData.Broadcast(PCMData);
				}
			}
		});
	}

	return NumSamples;
}

void UImportedSoundWave::BeginDestroy()
{
	UE_LOG(AudioLog, Warning, TEXT("Imported sound wave ('%s') data will be cleared because it is being unloaded"), *GetName());

	Super::BeginDestroy();
}

void UImportedSoundWave::Parse(FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances)
{
	FRAIScopeLock Lock(&*DataGuard);

	if (ActiveSound.PlaybackTime == 0.f)
	{
		UE_LOG(AudioLog, Log, TEXT("The playback time for the sound wave '%s' will be set to '%f'"), *GetName(), ParseParams.StartTime);
		RewindPlaybackTime_Internal(ParseParams.StartTime);
	}

	// Stopping all other active sounds that are using the same sound wave, so that only one sound wave can be played at a time
	const TArray<FActiveSound*>& ActiveSounds = AudioDevice->GetActiveSounds();
	for (FActiveSound* ActiveSoundPtr : ActiveSounds)
	{
		if (ActiveSoundPtr->GetSound() == this && ActiveSoundPtr->IsPlayingAudio() && &ActiveSound != ActiveSoundPtr)
		{
			UE_LOG(AudioLog, Warning, TEXT("Stopping the active sound '%s' because it is using the same sound wave '%s' (only one imported sound wave can be played at a time)"), *ActiveSoundPtr->GetOwnerName(), *GetName());
			AudioDevice->StopActiveSound(ActiveSoundPtr);
		}
	}

	ActiveSound.PlaybackTime = GetPlaybackTime_Internal();

	if (IsPlaybackFinished_Internal())
	{
		if (!PlaybackFinishedBroadcast)
		{
			UE_LOG(AudioLog, Warning, TEXT("Playback of the sound wave '%s' has been completed"), *GetName());

			PlaybackFinishedBroadcast = true;

			AsyncTask(ENamedThreads::GameThread, [WeakThis = MakeWeakObjectPtr(this)] ()
			{
				if (WeakThis.IsValid())
				{
					if (WeakThis->OnAudioPlaybackFinishedNative.IsBound())
					{
						WeakThis->OnAudioPlaybackFinishedNative.Broadcast();
					}

					if (WeakThis->OnAudioPlaybackFinished.IsBound())
					{
						WeakThis->OnAudioPlaybackFinished.Broadcast();
					}
				}
			});
		}

		if (!bLooping)
		{
			if (bStopSoundOnPlaybackFinish)
			{
				UE_LOG(AudioLog, Log, TEXT("Playback of the sound wave '%s' has reached the end and will be stopped"), *GetName());
				AudioDevice->StopActiveSound(&ActiveSound);
			}
		}
		else
		{
			UE_LOG(AudioLog, Log, TEXT("The sound wave '%s' will be looped"), *GetName());
			ActiveSound.PlaybackTime = 0.f;
			RewindPlaybackTime_Internal(0.f);
		}
	}

	Super::Parse(AudioDevice, NodeWaveInstanceHash, ActiveSound, ParseParams, WaveInstances);
}

void UImportedSoundWave::PopulateAudioDataFromDecodedInfo(FDecodedAudioStruct&& DecodedAudioInfo)
{
	FRAIScopeLock Lock(&*DataGuard);

	// If the sound wave has not yet been filled in with audio data and the initial desired sample rate and the number of channels are set, resample and mix the channels
	if (InitialDesiredSampleRate.IsSet() || InitialDesiredNumOfChannels.IsSet())
	{
		URuntimeAudioImporterLibrary::ResampleAndMixChannelsInDecodedInfo(DecodedAudioInfo,
			InitialDesiredSampleRate.IsSet() ? InitialDesiredSampleRate.GetValue() : DecodedAudioInfo.SoundWaveBasicInfo.SampleRate,
			InitialDesiredNumOfChannels.IsSet() ? InitialDesiredNumOfChannels.GetValue() : DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels);
	}

	const FString DecodedAudioInfoString = DecodedAudioInfo.ToString();

	Duration = DecodedAudioInfo.SoundWaveBasicInfo.Duration;
	SetImportedSampleRate(0);
	SetSampleRate(DecodedAudioInfo.SoundWaveBasicInfo.SampleRate);
	NumChannels = DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels;
	ImportedAudioFormat = DecodedAudioInfo.SoundWaveBasicInfo.AudioFormat;

	PCMBufferInfo->PCMData = MoveTemp(DecodedAudioInfo.PCMInfo.PCMData);
	PCMBufferInfo->PCMNumOfFrames = DecodedAudioInfo.PCMInfo.PCMNumOfFrames;

	{
		const bool IsBound = [this] ()
			{
				FRAIScopeLock Lock(&OnPopulateAudioData_DataGuard);
				return OnPopulateAudioDataNative.IsBound() || OnPopulateAudioData.IsBound();
			}();
			if (IsBound)
			{
				TArray<float> PCMData(PCMBufferInfo->PCMData.GetView().GetData(), PCMBufferInfo->PCMData.GetView().Num());
				AsyncTask(ENamedThreads::GameThread, [WeakThis = MakeWeakObjectPtr(this), PCMData = MoveTemp(PCMData)] () mutable
				{
					if (WeakThis.IsValid())
					{
						FRAIScopeLock Lock(&WeakThis->OnPopulateAudioData_DataGuard);
						if (WeakThis->OnPopulateAudioDataNative.IsBound())
						{
							WeakThis->OnPopulateAudioDataNative.Broadcast(PCMData);
						}
						if (WeakThis->OnPopulateAudioData.IsBound())
						{
							WeakThis->OnPopulateAudioData.Broadcast(PCMData);
						}
					}
					else
					{
						UE_LOG(AudioLog, Warning, TEXT("Unable to broadcast OnPopulateAudioDataNative and OnPopulateAudioData delegates because the streaming sound wave has been destroyed"));
					}
				});
			}
	}

	{
		const bool IsBound = [this] ()
			{
				FRAIScopeLock Lock(&OnPopulateAudioData_DataGuard);
				return OnPopulateAudioStateNative.IsBound() || OnPopulateAudioState.IsBound();
			}();
			if (IsBound)
			{
				AsyncTask(ENamedThreads::GameThread, [WeakThis = MakeWeakObjectPtr(this)] ()
				{
					if (WeakThis.IsValid())
					{
						FRAIScopeLock Lock(&WeakThis->OnPopulateAudioData_DataGuard);
						WeakThis->OnPopulateAudioStateNative.Broadcast();
						WeakThis->OnPopulateAudioState.Broadcast();
					}
					else
					{
						UE_LOG(AudioLog, Warning, TEXT("Unable to broadcast OnPopulateAudioStateNative and OnPopulateAudioState delegates because the streaming sound wave has been destroyed"));
					}
				});
			}
	}

	UE_LOG(AudioLog, Log, TEXT("The audio data has been populated successfully. Information about audio data:\n%s"), *DecodedAudioInfoString);
}

void UImportedSoundWave::ReleaseMemory()
{
	FRAIScopeLock Lock(&*DataGuard);
	UE_LOG(AudioLog, Warning, TEXT("Releasing memory for the sound wave '%s'"), *GetName());
	PCMBufferInfo->PCMData.Empty();
	PCMBufferInfo->PCMNumOfFrames = 0;
	Duration = 0;
}

void UImportedSoundWave::SetLooping(bool bLoop)
{
	bLooping = bLoop;
}

void UImportedSoundWave::SetSubtitles(const TArray<FEditableSubtitleCue>& InSubtitles)
{
	Subtitles.Empty();

	for (const FEditableSubtitleCue& Subtitle : InSubtitles)
	{
		FSubtitleCue ConvertedSubtitle;
		{
			ConvertedSubtitle.Text = Subtitle.Text;
			ConvertedSubtitle.Time = Subtitle.Time;
		}

		Subtitles.Add(ConvertedSubtitle);
	}
}

void UImportedSoundWave::SetVolume(float InVolume)
{
	Volume = InVolume;
}

void UImportedSoundWave::SetPitch(float InPitch)
{
	Pitch = InPitch;
}

bool UImportedSoundWave::RewindPlaybackTime(float PlaybackTime)
{
	FRAIScopeLock Lock(&*DataGuard);
	return RewindPlaybackTime_Internal(PlaybackTime);
}

bool UImportedSoundWave::RewindPlaybackTime_Internal(float PlaybackTime)
{
	if (PlaybackTime > Duration)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to rewind playback time for the imported sound wave '%s' by time '%f' because total length is '%f'"), *GetName(), PlaybackTime, Duration);
		return false;
	}

	return SetNumOfPlayedFrames_Internal(PlaybackTime * SampleRate);
}

bool UImportedSoundWave::SetInitialDesiredSampleRate(int32 DesiredSampleRate)
{
	if (PCMBufferInfo->PCMData.GetView().Num() > 0)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to set the initial desired sample rate for the imported sound wave '%s' to '%d' because the PCM data has already been populated"), *GetName(), DesiredSampleRate);
		return false;
	}

	if (DesiredSampleRate <= 0)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to set the initial desired sample rate for the imported sound wave '%s' to '%d' because the sample rate must be greater than zero"), *GetName(), DesiredSampleRate);
		return false;
	}

	InitialDesiredSampleRate = DesiredSampleRate;
	SampleRate = DesiredSampleRate;
	UE_LOG(AudioLog, Log, TEXT("Successfully set the initial desired sample rate for the imported sound wave '%s' to '%d'"), *GetName(), DesiredSampleRate);
	return true;
}

bool UImportedSoundWave::SetInitialDesiredNumOfChannels(int32 DesiredNumOfChannels)
{
	if (PCMBufferInfo->PCMData.GetView().Num() > 0)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to set the initial desired number of channels for the imported sound wave '%s' to '%d' because the PCM data has already been populated"), *GetName(), DesiredNumOfChannels);
		return false;
	}

	if (DesiredNumOfChannels <= 0)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to set the initial desired number of channels for the imported sound wave '%s' to '%d' because the number of channels must be greater than zero"), *GetName(), DesiredNumOfChannels);
		return false;
	}

	InitialDesiredNumOfChannels = DesiredNumOfChannels;
	NumChannels = DesiredNumOfChannels;
	UE_LOG(AudioLog, Log, TEXT("Successfully set the initial desired number of channels for the imported sound wave '%s' to '%d'"), *GetName(), DesiredNumOfChannels);
	return true;
}

void UImportedSoundWave::StopPlayback(const UObject* WorldContextObject, const FOnStopPlaybackResult& Result)
{
	StopPlayback(WorldContextObject, FOnStopPlaybackResultNative::CreateWeakLambda(this, [Result] (bool bSucceeded)
		{
			Result.ExecuteIfBound(bSucceeded);
		}));
}

void UImportedSoundWave::StopPlayback(const UObject* WorldContextObject, const FOnStopPlaybackResultNative& Result)
{
	if (!IsInAudioThread())
	{
		FAudioThread::RunCommandOnAudioThread([WeakThis = MakeWeakObjectPtr(this), WorldContextObject, Result] ()
		{
			WeakThis->StopPlayback(WorldContextObject, Result);
		});
		return;
	}

	auto ExecuteResult = [Result] (bool bSucceeded)
		{
			AsyncTask(ENamedThreads::GameThread, [Result, bSucceeded] ()
			{
				Result.ExecuteIfBound(bSucceeded);
			});
		};

	if (!GEngine)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to stop the playback of the sound wave '%s' because GEngine is invalid"), *GetName());
		ExecuteResult(false);
		return;
	}

	UWorld* ThisWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!ThisWorld)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to stop the playback of the sound wave '%s' because the world context object is invalid"), *GetName());
		ExecuteResult(false);
		return;
	}

	if (FAudioDeviceHandle AudioDevice = ThisWorld->GetAudioDevice())
	{
		const TArray<FActiveSound*>& ActiveSounds = AudioDevice->GetActiveSounds();
		for (FActiveSound* ActiveSoundPtr : ActiveSounds)
		{
			if (ActiveSoundPtr->GetSound() == this && ActiveSoundPtr->IsPlayingAudio())
			{
				UE_LOG(AudioLog, Warning, TEXT("Stopping the active sound '%s' playing the sound wave '%s'"), *ActiveSoundPtr->GetOwnerName(), *GetName());
				AudioDevice->StopActiveSound(ActiveSoundPtr);

				// Only one sound wave can be played at a time, so we can stop the loop here
				ExecuteResult(true);
				return;
			}
		}
	}

	UE_LOG(AudioLog, Warning, TEXT("The sound wave '%s' is not playing"), *GetName());
	ExecuteResult(true);
}

bool UImportedSoundWave::SetNumOfPlayedFrames(uint32 NumOfFrames)
{
	FRAIScopeLock Lock(&*DataGuard);
	return SetNumOfPlayedFrames_Internal(NumOfFrames);
}

bool UImportedSoundWave::SetNumOfPlayedFrames_Internal(uint32 NumOfFrames)
{
	if (NumOfFrames < 0 || NumOfFrames > PCMBufferInfo->PCMNumOfFrames)
	{
		UE_LOG(AudioLog, Error, TEXT("Cannot change the current frame for the imported sound wave '%s' to frame '%d' because the total number of frames is '%d'"), *GetName(), NumOfFrames, PCMBufferInfo->PCMNumOfFrames);
		return false;
	}

	PlayedNumOfFrames = NumOfFrames;

	ResetPlaybackFinish();

	return true;
}

uint32 UImportedSoundWave::GetNumOfPlayedFrames() const
{
	FRAIScopeLock Lock(&*DataGuard);
	return GetNumOfPlayedFrames_Internal();
}

uint32 UImportedSoundWave::GetNumOfPlayedFrames_Internal() const
{
	return PlayedNumOfFrames;
}

float UImportedSoundWave::GetPlaybackTime() const
{
	FRAIScopeLock Lock(&*DataGuard);
	return GetPlaybackTime_Internal();
}

float UImportedSoundWave::GetPlaybackTime_Internal() const
{
	if (GetNumOfPlayedFrames() == 0 || SampleRate <= 0)
	{
		return 0;
	}

	return static_cast<float>(GetNumOfPlayedFrames()) / SampleRate;
}

float UImportedSoundWave::GetDurationConst() const
{
	FRAIScopeLock Lock(&*DataGuard);
	return GetDurationConst_Internal();
}

float UImportedSoundWave::GetDurationConst_Internal() const
{
	return Duration;
}

float UImportedSoundWave::GetDuration() const
{
	return GetDurationConst();
}

int32 UImportedSoundWave::GetSampleRate() const
{
	return SampleRate;
}

int32 UImportedSoundWave::GetNumOfChannels() const
{
	return NumChannels;
}

int32 UImportedSoundWave::GetNumberOfChannels() const
{
	return GetNumOfChannels();
}

float UImportedSoundWave::GetPlaybackPercentage() const
{
	FRAIScopeLock Lock(&*DataGuard);

	if (GetNumOfPlayedFrames_Internal() == 0 || PCMBufferInfo->PCMNumOfFrames == 0)
	{
		return 0;
	}

	return static_cast<float>(GetNumOfPlayedFrames_Internal()) / PCMBufferInfo->PCMNumOfFrames * 100;
}

bool UImportedSoundWave::IsPlaybackFinished() const
{
	FRAIScopeLock Lock(&*DataGuard);
	return IsPlaybackFinished_Internal();
}

bool UImportedSoundWave::IsPlaying(const UObject* WorldContextObject) const
{
	// Audio device operations must be done on the audio thread
	if (!IsInAudioThread())
	{
		TPromise<bool> PromiseResult;

		Async(EAsyncExecution::TaskGraph, [WeakThis = MakeWeakObjectPtr(this), WorldContextObject, &PromiseResult] ()
		{
			FAudioThread::RunCommandOnAudioThread([WeakThis, WorldContextObject, &PromiseResult] ()
			{
				if (WeakThis.IsValid())
				{
					PromiseResult.SetValue(WeakThis->IsPlaying(WorldContextObject));
					return;
				}
				PromiseResult.SetValue(false);
			});
		}).Wait();

		TFuture<bool> FutureResult = PromiseResult.GetFuture();

		// This operation is very short-lasting, so it's fine to wait here, even if it's called from the game thread every frame
		FutureResult.Wait();
		return FutureResult.Get();
	}

	if (!GEngine)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to check if the sound wave '%s' is playing because GEngine is invalid"), *GetName());
		return false;
	}

	UWorld* ThisWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!ThisWorld)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to check if the sound wave '%s' is playing because the world context object is invalid"), *GetName());
		return false;
	}

	if (FAudioDeviceHandle AudioDevice = ThisWorld->GetAudioDevice())
	{
		const TArray<FActiveSound*>& ActiveSounds = AudioDevice->GetActiveSounds();
		for (FActiveSound* ActiveSoundPtr : ActiveSounds)
		{
			if (ActiveSoundPtr->GetSound() == this && ActiveSoundPtr->IsPlayingAudio())
			{
				UE_LOG(AudioLog, Log, TEXT("The sound wave '%s' is playing by the owner '%s' and audio component '%s'"), *GetName(), *ActiveSoundPtr->GetOwnerName(), *ActiveSoundPtr->GetAudioComponentName());
				return true;
			}
		}
	}
	UE_LOG(AudioLog, Log, TEXT("The sound wave '%s' is not playing"), *GetName());
	return false;
}

bool UImportedSoundWave::IsPlaybackFinished_Internal() const
{
	// Are there enough frames for future playback from the current ones or not
	const bool bOutOfFrames = GetNumOfPlayedFrames_Internal() >= PCMBufferInfo->PCMNumOfFrames;

	// Is PCM data valid
	const bool bValidPCMData = PCMBufferInfo.IsValid();

	return bOutOfFrames || !bValidPCMData;
}

bool UImportedSoundWave::GetAudioHeaderInfo(FRuntimeAudioHeaderInfo& HeaderInfo) const
{
	FRAIScopeLock Lock(&*DataGuard);

	if (!PCMBufferInfo.IsValid())
	{
		UE_LOG(AudioLog, Error, TEXT("Failed to retrieve audio header information due to an invalid PCM buffer"));
		return false;
	}

	{
		HeaderInfo.Duration = GetDurationConst_Internal();
		HeaderInfo.AudioFormat = GetAudioFormat();
		HeaderInfo.SampleRate = GetSampleRate();
		HeaderInfo.NumOfChannels = GetNumOfChannels();
		HeaderInfo.PCMDataSize = PCMBufferInfo->PCMData.GetView().Num();
	}

	return true;
}

void UImportedSoundWave::ResetPlaybackFinish()
{
	PlaybackFinishedBroadcast = false;
}

TArray<float> UImportedSoundWave::GetPCMBufferCopy()
{
	FRAIScopeLock Lock(&*DataGuard);
	return TArray<float>(PCMBufferInfo.Get()->PCMData.GetView().GetData(), PCMBufferInfo.Get()->PCMData.GetView().Num());
}

const FPCMStruct& UImportedSoundWave::GetPCMBuffer() const
{
	return *PCMBufferInfo.Get();
}

ERuntimeAudioFormat UImportedSoundWave::GetAudioFormat() const
{
	return ImportedAudioFormat;
}