// Georgy Treshchev 2023.

#include "RuntimeAudioImporter/CapturableSoundWave.h"

#include "RuntimeAudioImporter/AudioStructs.h"
#include "AudioThread.h"
#include "Async/Async.h"
#include "UObject/WeakObjectPtrTemplates.h"

UCapturableSoundWave::UCapturableSoundWave(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCapturableSoundWave::BeginDestroy()
{
	AudioCapture.AbortStream();
	AudioCapture.CloseStream();

	Super::BeginDestroy();
}

UCapturableSoundWave* UCapturableSoundWave::CreateCapturableSoundWave()
{
	if (!IsInGameThread())
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to create a sound wave outside of the game thread"));
		return nullptr;
	}

	return NewObject<UCapturableSoundWave>();
}

void UCapturableSoundWave::GetAvailableAudioInputDevices(const FOnGetAvailableAudioInputDevicesResult& Result)
{
	GetAvailableAudioInputDevices(FOnGetAvailableAudioInputDevicesResultNative::CreateLambda([Result] (const TArray<FRuntimeAudioInputDeviceInfo>& AvailableDevices)
		{
			Result.ExecuteIfBound(AvailableDevices);
		}));
}

void UCapturableSoundWave::GetAvailableAudioInputDevices(const FOnGetAvailableAudioInputDevicesResultNative& Result)
{
	if (!IsInAudioThread())
	{
		FAudioThread::RunCommandOnAudioThread([Result] ()
		{
			GetAvailableAudioInputDevices(Result);
		});
		return;
	}

	auto ExecuteResult = [Result] (const TArray<FRuntimeAudioInputDeviceInfo>& AvailableDevices)
		{
			FAudioThread::RunCommandOnGameThread([Result, AvailableDevices] ()
			{
				Result.ExecuteIfBound(AvailableDevices);
			});
		};

	TArray<FRuntimeAudioInputDeviceInfo> AvailableDevices;

	Audio::FAudioCapture AudioCapture;
	TArray<Audio::FCaptureDeviceInfo> InputDevices;

	AudioCapture.GetCaptureDevicesAvailable(InputDevices);

	for (const Audio::FCaptureDeviceInfo& CaptureDeviceInfo : InputDevices)
	{
		AvailableDevices.Add(FRuntimeAudioInputDeviceInfo(CaptureDeviceInfo));
	}

	ExecuteResult(AvailableDevices);
}

bool UCapturableSoundWave::StartCapture(int32 DeviceId)
{
	Audio::FAudioCaptureDeviceParams Params = Audio::FAudioCaptureDeviceParams();
	Params.DeviceIndex = DeviceId;
	LastDeviceIndex = DeviceId;

	Audio::FOnAudioCaptureFunction
		OnCapture = [WeakThis = MakeWeakObjectPtr(this)](const void* PCMData, int32 NumFrames, int32 NumOfChannels,
						   int32 InSampleRate,
						   double StreamTime, bool bOverFlow)
		{
			if (!WeakThis.IsValid())
			{
				UE_LOG(AudioLog, Error, TEXT("Unable to capture audio using a capturable sound wave as it has been destroyed"));
				return;
			}

			if (WeakThis->AudioCapture.IsCapturing())
			{
				const int64 PCMDataSize = NumOfChannels * NumFrames;
				int64 PCMDataSizeInBytes = PCMDataSize * sizeof(float);

				if (PCMDataSizeInBytes > TNumericLimits<int32>::Max())
				{
					UE_LOG(AudioLog, Warning, TEXT("Unable to append audio data as the size of the data exceeds the maximum size of int32 (PCMDataSizeInBytes: %lld, Max: %d)"), PCMDataSizeInBytes, TNumericLimits<int32>::Max());
					PCMDataSizeInBytes = TNumericLimits<int32>::Max();
				}

				WeakThis->AppendAudioDataFromRAW(TArray<uint8>(reinterpret_cast<const uint8*>(PCMData), static_cast<int32>(PCMDataSizeInBytes)), ERuntimeRAWAudioFormat::Float32,
					InSampleRate
				  , NumOfChannels);
			}
		};

	if (AudioCapture.IsStreamOpen())
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to start capture as the stream is already open"));
		return false;
	}

	if (!AudioCapture.
		OpenAudioCaptureStream

		(Params, MoveTemp(OnCapture), 1024))
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to open capturing stream for sound wave %s"), *GetName());
		return false;
	}

	if (!AudioCapture.StartStream())
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to start capturing for sound wave %s"), *GetName());
		return false;
	}

	UE_LOG(AudioLog, Log, TEXT("Successfully started capturing for sound wave %s"), *GetName());
	return true;
}

void UCapturableSoundWave::StopCapture()
{
	if (AudioCapture.IsStreamOpen())
	{
		AudioCapture.CloseStream();
	}
}

bool UCapturableSoundWave::ToggleMute(bool bMute)
{
	if (bMute)
	{
		StopCapture();
		return true;
	}
	else
	{
		return StartCapture(LastDeviceIndex);
	}
}

bool UCapturableSoundWave::IsCapturing() const
{
	return AudioCapture.IsCapturing();
}