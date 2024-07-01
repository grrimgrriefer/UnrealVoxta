// Georgy Treshchev 2024.

#include "AudioImporter.h"
#include "RuntimeCodecFactory.h"
#include "ImportedSoundWave.h"
#include "RAW_RuntimeCodec.h"

void UAudioImporter::ImportAudioFromBuffer(FString identifier, TArray64<uint8> AudioData)
{
	if (IsInGameThread())
	{
		AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [WeakThis = MakeWeakObjectPtr(this), ID = identifier, AudioData = MoveTemp(AudioData)] () mutable
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ImportAudioFromBuffer(ID, MoveTemp(AudioData));
			}
			else
			{
				UE_LOG(AudioLog, Error, TEXT("Failed to import audio from buffer because the RuntimeAudioImporterLibrary object has been destroyed"));
			}
		});
		return;
	}

	FEncodedAudioStruct EncodedAudioInfo(AudioData, ERuntimeAudioFormat::Wav);

	FDecodedAudioStruct DecodedAudioInfo;
	if (!DecodeAudioData(MoveTemp(EncodedAudioInfo), DecodedAudioInfo))
	{
		UE_LOG(AudioLog, Error, TEXT("Failed to decode audiodata: FailedToReadAudioDataArray"));
		return;
	}

	ImportAudioFromDecodedInfo(identifier, MoveTemp(DecodedAudioInfo));
}

bool UAudioImporter::DecodeAudioData(FEncodedAudioStruct&& EncodedAudioInfo, FDecodedAudioStruct& DecodedAudioInfo)
{
	FRuntimeCodecFactory CodecFactory;
	TArray<FBaseRuntimeCodec*> RuntimeCodecs = [&EncodedAudioInfo, &CodecFactory] ()
		{
			if (EncodedAudioInfo.AudioFormat == ERuntimeAudioFormat::Auto)
			{
				return CodecFactory.GetCodecs(EncodedAudioInfo.AudioData);
			}
			return CodecFactory.GetCodecs(EncodedAudioInfo.AudioFormat);
		}();

		for (FBaseRuntimeCodec* RuntimeCodec : RuntimeCodecs)
		{
			EncodedAudioInfo.AudioFormat = RuntimeCodec->GetAudioFormat();
			if (!RuntimeCodec->Decode(MoveTemp(EncodedAudioInfo), DecodedAudioInfo))
			{
				UE_LOG(AudioLog, Error, TEXT("Something went wrong while decoding '%s' audio data"), *UEnum::GetValueAsString(EncodedAudioInfo.AudioFormat));
				continue;
			}
			return true;
		}

		UE_LOG(AudioLog, Error, TEXT("Failed to decode the audio data because the codec for the format '%s' was not found"), *UEnum::GetValueAsString(EncodedAudioInfo.AudioFormat));
		return false;
}

void UAudioImporter::ImportAudioFromDecodedInfo(FString identifier, FDecodedAudioStruct&& DecodedAudioInfo)
{
	// Making sure we are in the game thread
	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, [WeakThis = MakeWeakObjectPtr(this), ID = identifier, DecodedAudioInfo = MoveTemp(DecodedAudioInfo)] () mutable
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ImportAudioFromDecodedInfo(ID, MoveTemp(DecodedAudioInfo));
			}
			else
			{
				UE_LOG(AudioLog, Error, TEXT("Unable to import audio from decoded info '%s' because the RuntimeAudioImporterLibrary object has been destroyed"), *DecodedAudioInfo.ToString());
			}
		});
		return;
	}

	UImportedSoundWave* ImportedSoundWave = NewObject<UImportedSoundWave>();
	if (!ImportedSoundWave)
	{
		UE_LOG(AudioLog, Error, TEXT("Something went wrong while creating the imported sound wave"));
		return;
	}

	ImportedSoundWave->AddToRoot();

	ImportedSoundWave->PopulateAudioDataFromDecodedInfo(MoveTemp(DecodedAudioInfo));

	UE_LOG(AudioLog, Log, TEXT("The audio data was successfully imported"));
	OnResult_Internal(identifier, ImportedSoundWave);

	ImportedSoundWave->RemoveFromRoot();
}

bool UAudioImporter::ResampleAndMixChannelsInDecodedInfo(FDecodedAudioStruct& DecodedAudioInfo, uint32 NewSampleRate, uint32 NewNumOfChannels)
{
	if (DecodedAudioInfo.SoundWaveBasicInfo.SampleRate <= 0 || NewSampleRate <= 0)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to resample audio data because the sample rate is invalid (Current: %d, New: %d)"), DecodedAudioInfo.SoundWaveBasicInfo.SampleRate, NewSampleRate);
		return false;
	}

	if (DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels <= 0 || NewNumOfChannels <= 0)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to mix audio data because the number of channels is invalid (Current: %d, New: %d)"), DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels, NewNumOfChannels);
		return false;
	}

	if (NewSampleRate == DecodedAudioInfo.SoundWaveBasicInfo.SampleRate && NewNumOfChannels == DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels)
	{
		UE_LOG(AudioLog, Log, TEXT("No need to resample or mix audio data"));
		return true;
	}

	Audio::FAlignedFloatBuffer WaveData(DecodedAudioInfo.PCMInfo.PCMData.GetView().GetData(), DecodedAudioInfo.PCMInfo.PCMData.GetView().Num());

	// Resampling if needed
	if (NewSampleRate != DecodedAudioInfo.SoundWaveBasicInfo.SampleRate)
	{
		Audio::FAlignedFloatBuffer ResamplerOutputData;
		if (!FRAW_RuntimeCodec::ResampleRAWData(WaveData, DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels, DecodedAudioInfo.SoundWaveBasicInfo.SampleRate, NewSampleRate, ResamplerOutputData))
		{
			UE_LOG(AudioLog, Error, TEXT("Unable to resample audio data to the sound wave's sample rate. Resampling failed"));
			return false;
		}
		WaveData = MoveTemp(ResamplerOutputData);
		DecodedAudioInfo.SoundWaveBasicInfo.SampleRate = NewSampleRate;
		UE_LOG(AudioLog, Log, TEXT("Audio data has been resampled to the desired sample rate '%d'"), NewSampleRate);
	}

	// Mixing the channels if needed
	if (NewNumOfChannels != DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels)
	{
		Audio::FAlignedFloatBuffer WaveDataTemp;
		if (!FRAW_RuntimeCodec::MixChannelsRAWData(WaveData, NewSampleRate, DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels, NewNumOfChannels, WaveDataTemp))
		{
			UE_LOG(AudioLog, Error, TEXT("Unable to mix audio data to the sound wave's number of channels. Mixing failed"));
			return false;
		}
		WaveData = MoveTemp(WaveDataTemp);
		DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels = NewNumOfChannels;
		UE_LOG(AudioLog, Log, TEXT("Audio data has been mixed to the desired number of channels '%d'"), NewNumOfChannels);
	}

	DecodedAudioInfo.PCMInfo.PCMData = FRuntimeBulkDataBuffer<float>(WaveData);
	return true;
}

void UAudioImporter::OnResult_Internal(FString identifier, UImportedSoundWave* ImportedSoundWave)
{
	// Making sure we are in the game thread
	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, [WeakThis = MakeWeakObjectPtr(this), ID = identifier, ImportedSoundWave] ()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->OnResult_Internal(ID, ImportedSoundWave);
			}
			else
			{
				UE_LOG(AudioLog, Error, TEXT("Unable to broadcast the result of the import because the RuntimeAudioImporterLibrary object has been destroyed"));
			}
		});
		return;
	}

	bool bBroadcasted{ false };

	if (AudioImportedEvent.IsBound())
	{
		bBroadcasted = true;
		AudioImportedEvent.Broadcast(identifier, ImportedSoundWave);
	}

	if (!bBroadcasted)
	{
		UE_LOG(AudioLog, Error, TEXT("You did not bind to the delegate to get the result of the import"));
	}
}