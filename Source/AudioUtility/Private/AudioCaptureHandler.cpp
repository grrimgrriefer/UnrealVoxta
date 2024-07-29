// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AudioCaptureHandler.h"
#include "VoiceRunnerThread.h"

void AudioCaptureHandler::RegisterSocket(TSharedPtr<AudioWebSocket> socket, int bufferMillisecondSize)
{
	m_webSocket = socket;
	m_bufferMillisecondSize = bufferMillisecondSize;
}

bool AudioCaptureHandler::TryInitialize(int32 samplerate, int32 numchannels)
{
	if (m_voiceCaptureDevice)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to start capture as the stream is already open"));
		return false;
	}

	FVoiceModule& VoiceModule = FVoiceModule::Get();
	if (VoiceModule.IsVoiceEnabled())
	{
		static IConsoleVariable* SilenceDetectionReleaseCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.MicNoiseGateThreshold"));
		static IConsoleVariable* SilenceDetectionThresholdCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.SilenceDetectionThreshold"));
		static IConsoleVariable* micInputGain = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.MicInputGain"));

		SilenceDetectionReleaseCVar->Set(0.001f);
		SilenceDetectionThresholdCVar->Set(0.001f);
		micInputGain->Set(6.0f);

		Audio::FAudioCapture AudioCapture;
		Audio::FCaptureDeviceInfo OutInfo;
		if (!AudioCapture.GetCaptureDeviceInfo(OutInfo, INDEX_NONE))
		{
			UE_LOG(AudioLog, Error, TEXT("Failed to fetch microhpone device information %s"), *m_deviceName);
			return false;
		}
		m_deviceName = OutInfo.DeviceName;
		UE_LOG(AudioLog, Log, TEXT("Using microphone device %s"), *m_deviceName);

		m_voiceCaptureDevice = VoiceModule.CreateVoiceCapture(m_deviceName, samplerate, numchannels);
	}
	if (m_voiceCaptureDevice.IsValid())
	{
		UE_LOG(AudioLog, Log, TEXT("Created voice capture"));
		return true;
	}
	else
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to create the voice caputre"));
		return false;
	}
}

bool AudioCaptureHandler::TryStartCapture()
{
	if (!m_voiceCaptureDevice.IsValid())
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to start capturing for sound wave, VoiceCapture pointer is invalid."));
		return false;
	}

	m_voiceRunnerThread = MakeUnique<FVoiceRunnerThread>(this, 0.15f);
	if (!m_voiceRunnerThread.IsValid())
	{
		UE_LOG(LogVoice, Error, TEXT("m_voiceRunnerThread is null! Func: %s; Line: %s"), *FString(__FUNCTION__), *FString::FromInt(__LINE__));
		return false;
	}

	if (!m_voiceCaptureDevice->Start())
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to start capturing for sound wave"));
		return false;
	}

	m_voiceRunnerThread->Start();

	UE_LOG(AudioLog, Log, TEXT("Successfully started voice capture & background thread"));
	m_isCapturing = true;
	return true;
}

void AudioCaptureHandler::StopCapture()
{
	if (m_voiceRunnerThread.IsValid())
	{
		m_voiceRunnerThread->Stop();
	}

	m_socketDataBuffer.Reset();
	m_isCapturing = false;
	m_voiceCaptureDevice->Stop();
}

void AudioCaptureHandler::ShutDown()
{
	StopCapture();
	m_voiceRunnerThread = nullptr;
}

void AudioCaptureHandler::CaptureVoiceInternal()
{
	if (!m_voiceCaptureDevice.IsValid())
	{
		UE_LOG(AudioLog, Warning, TEXT("VoiceCapture device has been destroyed, can't capture any more data."));
		return;
	}

	uint32 AvailableBytes = 0;
	auto CaptureState = m_voiceCaptureDevice->GetCaptureState(AvailableBytes);

	if (AvailableBytes < 1)
	{
		return;
	}

	m_socketDataBuffer.Reset();

	if (CaptureState == EVoiceCaptureState::Ok)
	{
		uint32 VoiceCaptureReadBytes = 0;

		m_socketDataBuffer.SetNumUninitialized(AvailableBytes);
		m_voiceCaptureDevice->GetVoiceData(
			m_socketDataBuffer.GetData(),
			AvailableBytes,
			VoiceCaptureReadBytes
		);
	}
}

void AudioCaptureHandler::SendInternal(const TArray<uint8> InData)
{
	if (InData.Num() > (m_bufferMillisecondSize * 32))
	{
		UE_LOG(LogTemp, Warning, TEXT("Client: cannot send data, too big: %d"), InData.Num());
		return;
	}
	m_webSocket->Send(InData.GetData(), InData.Num());
}

void AudioCaptureHandler::CaptureAndSendVoiceData()
{
	CaptureVoiceInternal();

	if (m_socketDataBuffer.Num() > 0)
	{
		SendInternal(m_socketDataBuffer);

		m_socketDataBuffer.Reset();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No data to send"));
	}
}

float AudioCaptureHandler::GetAmplitude() const
{
	if (m_voiceCaptureDevice.IsValid())
	{
		return m_voiceCaptureDevice->GetCurrentAmplitude();
	}
	return -1.f;
}

FString AudioCaptureHandler::GetDeviceName() const
{
	return m_deviceName;
}