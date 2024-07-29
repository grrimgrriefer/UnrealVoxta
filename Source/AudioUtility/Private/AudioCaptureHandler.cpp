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
	if (m_voiceCaptureDevice)
	{
		UE_LOGFMT(AudioLog, Error, "Unable to start capture as the stream is already open");
		return false;
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

	m_voiceRunnerThread = MakeUnique<FVoiceRunnerThread>(this, 0.15f);
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

void AudioCaptureHandler::CaptureVoiceInternal(TArray<uint8>& voiceDataBuffer) const
{
	if (!m_voiceCaptureDevice.IsValid())
	{
		UE_LOGFMT(AudioLog, Warning, "VoiceCapture device has been destroyed, can't capture any more data.");
		return;
	}

	uint32 AvailableBytes = 0;
	auto CaptureState = m_voiceCaptureDevice->GetCaptureState(AvailableBytes);

	if (AvailableBytes < 1)
	{
		return;
	}

	voiceDataBuffer.Reset();

	if (CaptureState == EVoiceCaptureState::Ok)
	{
		uint32 VoiceCaptureReadBytes = 0;

		voiceDataBuffer.SetNumUninitialized(AvailableBytes);
		m_voiceCaptureDevice->GetVoiceData(
			voiceDataBuffer.GetData(),
			AvailableBytes,
			VoiceCaptureReadBytes
		);
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
	CaptureVoiceInternal(m_socketDataBuffer);

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