// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AudioCaptureHandler.h"
#include "VoiceRunnerThread.h"
#include "Logging/StructuredLog.h"

void AudioCaptureHandler::RegisterSocket(TWeakPtr<AudioWebSocket> socket, int bufferMillisecondSize)
{
	m_webSocket = socket;
	m_bufferMillisecondSize = bufferMillisecondSize;
}

bool AudioCaptureHandler::TryInitializeVoiceCapture(int sampleRate, int numChannels)
{
	if (m_voiceCaptureDevice.IsValid())
	{
		UE_LOGFMT(AudioLog, Warning, "Skipped creating voice capture as it was already created earlier.");
		return true;
	}

	FVoiceModule& VoiceModule = FVoiceModule::Get();
	if (VoiceModule.IsVoiceEnabled())
	{
		ConfigureSilenceTresholds();

		Audio::FAudioCapture AudioCapture;
		Audio::FCaptureDeviceInfo OutInfo;
		if (!AudioCapture.GetCaptureDeviceInfo(OutInfo, INDEX_NONE))
		{
			UE_LOGFMT(AudioLog, Error, "Failed to fetch microhpone device information {0}", m_deviceName);
			return false;
		}
		m_deviceName = OutInfo.DeviceName;
		UE_LOGFMT(AudioLog, Log, "Using microphone device {0}", m_deviceName);

		m_voiceCaptureDevice = VoiceModule.CreateVoiceCapture(m_deviceName, sampleRate, numChannels);
	}
	if (m_voiceCaptureDevice.IsValid())
	{
		UE_LOGFMT(AudioLog, Log, "Created voice capture");
		return true;
	}
	else
	{
		UE_LOGFMT(AudioLog, Error, "Unable to create the voice caputre");
		return false;
	}
}

void AudioCaptureHandler::ConfigureSilenceTresholds(float micNoiseGateThreshold, float silenceDetectionThreshold, float micInputGain)
{
	static IConsoleVariable* SilenceDetectionReleaseCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.MicNoiseGateThreshold"));
	static IConsoleVariable* SilenceDetectionThresholdCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.SilenceDetectionThreshold"));
	static IConsoleVariable* MicInputGain = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.MicInputGain"));

	SilenceDetectionReleaseCVar->Set(micNoiseGateThreshold);
	SilenceDetectionThresholdCVar->Set(silenceDetectionThreshold);
	MicInputGain->Set(micInputGain);
}

bool AudioCaptureHandler::TryStartVoiceCapture()
{
	if (!m_voiceCaptureDevice.IsValid())
	{
		UE_LOGFMT(AudioLog, Error, "Unable to start capturing for sound wave, VoiceCapture pointer is invalid.");
		return false;
	}

	if (m_isCapturing)
	{
		UE_LOGFMT(AudioLog, Warning, "Voice capture is currently already capturing, ignoring new attempt to start.");
		return false;
	}

	// we only use 75% of the buffer, the extra 25% is space in case the thread encounters minor lag / delay.
	m_voiceRunnerThread = MakeUnique<FVoiceRunnerThread>(this, (m_bufferMillisecondSize / 1000.f * 0.75f));
	if (!m_voiceRunnerThread.IsValid())
	{
		UE_LOGFMT(LogVoice, Error, "VoiceRunnerThread has been destroyed.");
		return false;
	}

	if (!m_voiceCaptureDevice->Start())
	{
		UE_LOGFMT(AudioLog, Error, "Unable to start capturing for sound wave");
		return false;
	}

	m_voiceRunnerThread->Start();

	UE_LOGFMT(AudioLog, Log, "Successfully started voice capture & background thread");
	m_isCapturing = true;
	return true;
}

void AudioCaptureHandler::StopCapture()
{
	FScopeLock Lock(&m_captureGuard);
	if (m_voiceRunnerThread.IsValid())
	{
		m_voiceRunnerThread->Stop();
	}

	m_socketDataBuffer.Reset();
	m_isCapturing = false;
	if (m_voiceCaptureDevice != nullptr)
	{
		m_voiceCaptureDevice->Stop();
	}
}

void AudioCaptureHandler::ShutDown()
{
	StopCapture();
	FScopeLock Lock(&m_captureGuard);
	m_voiceRunnerThread = nullptr;
}

void AudioCaptureHandler::CaptureVoiceInternal(TArray<uint8>& voiceDataBuffer, float& decibels) const
{
	if (!m_voiceCaptureDevice.IsValid())
	{
		UE_LOGFMT(AudioLog, Warning, "VoiceCapture device has been destroyed, can't capture any more data.");
		return;
	}

	uint32 AvailableBytes = 0;

	if (m_voiceCaptureDevice->GetCaptureState(AvailableBytes) == EVoiceCaptureState::Ok)
	{
		if (AvailableBytes < 1)
		{
			return;
		}
		voiceDataBuffer.Reset();

		uint32 VoiceCaptureReadBytes = 0;

		voiceDataBuffer.SetNumUninitialized(AvailableBytes);
		m_voiceCaptureDevice->GetVoiceData(
			voiceDataBuffer.GetData(),
			AvailableBytes,
			VoiceCaptureReadBytes
		);

		decibels = AnalyseDecibels(voiceDataBuffer, VoiceCaptureReadBytes);
	}
}

void AudioCaptureHandler::SendInternal(const TArray<uint8> rawData) const
{
	if (rawData.Num() > (m_bufferMillisecondSize * 32))
	{
		UE_LOGFMT(LogTemp, Warning, "Client: cannot send data, too big: {0}", rawData.Num());
		return;
	}
	m_webSocket.Pin()->Send(rawData.GetData(), rawData.Num());
}

void AudioCaptureHandler::CaptureAndSendVoiceData()
{
	FScopeLock Lock(&m_captureGuard);
	CaptureVoiceInternal(m_socketDataBuffer, m_decibels);

	if (m_socketDataBuffer.Num() > 0)
	{
		SendInternal(m_socketDataBuffer);

		m_socketDataBuffer.Reset();
	}
	else
	{
		UE_LOGFMT(LogTemp, Warning, "No data to send");
	}
}

float AudioCaptureHandler::GetDecibels() const
{
	FScopeLock Lock(&m_captureGuard);
	if (m_voiceCaptureDevice.IsValid() && m_isCapturing)
	{
		return m_decibels;
	}
	return -100.f;
}

FString AudioCaptureHandler::GetDeviceName() const
{
	return m_deviceName;
}

float AudioCaptureHandler::AnalyseDecibels(const TArray<uint8>& VoiceData, uint32 DataSize) const
{
	int16 Sample;
	float SumSquared = 0.0f;

	for (uint32 i = 0; i < DataSize / 2; ++i)
	{
		Sample = (VoiceData[i * 2 + 1] << 8) | VoiceData[i * 2];
		SumSquared += static_cast<float>(Sample) * static_cast<float>(Sample);
	}

	float MeanSquared = SumSquared / (DataSize / 2.f);
	float RootMeanSquare = FMath::Sqrt(MeanSquared);
	float Decibels = 20.0f * FMath::LogX(10.0f, RootMeanSquare / 32768.0f);

	return Decibels;
}