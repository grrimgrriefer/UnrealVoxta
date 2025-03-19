// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AudioCaptureHandler.h"
#include "VoxtaDefines.h"
#include "VoiceRunnerThread.h"
#include "Logging/StructuredLog.h"
#include "AudioWebSocket.h"
#include "Voice.h"
#include "AudioCaptureCore.h"
#include "Math/UnrealMathUtility.h"

#ifndef VOXTA_LOG_DEFINED
DEFINE_LOG_CATEGORY(VoxtaLog);
#define VOXTA_LOG_DEFINED
#endif

void AudioCaptureHandler::RegisterSocket(TWeakPtr<AudioWebSocket> socket, int bufferMillisecondSize)
{
	m_webSocket = socket;
	m_bufferMillisecondSize = bufferMillisecondSize;
}

bool AudioCaptureHandler::TryInitializeVoiceCapture(int sampleRate, int numChannels)
{
	if (m_voiceCaptureDevice.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Warning, "Skipped creating voice capture as it was already created earlier.");
		return true;
	}

	FVoiceModule& VoiceModule = FVoiceModule::Get();
	if (VoiceModule.IsVoiceEnabled())
	{
		Audio::FAudioCapture AudioCapture;
		Audio::FCaptureDeviceInfo OutInfo;
		if (!AudioCapture.GetCaptureDeviceInfo(OutInfo, INDEX_NONE))
		{
			UE_LOGFMT(VoxtaLog, Error, "Failed to fetch microhpone device information {0}", m_deviceName);
			return false;
		}
		m_deviceName = OutInfo.DeviceName;
		UE_LOGFMT(VoxtaLog, Log, "Using microphone device {0}", m_deviceName);

		m_voiceCaptureDevice = VoiceModule.CreateVoiceCapture(m_deviceName, sampleRate, numChannels);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Voice module is not enabled, cannot start Microphone input.");
	}
	if (m_voiceCaptureDevice.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Log, "Created voice capture");
		return true;
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Unable to create the voice caputre");
		return false;
	}
}

void AudioCaptureHandler::ConfigureSilenceTresholds(float micNoiseGateThreshold, float silenceDetectionThreshold, float micInputGain)
{
	static IConsoleVariable* silenceDetectionReleaseCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.MicNoiseGateThreshold"));
	static IConsoleVariable* silenceDetectionThresholdCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.SilenceDetectionThreshold"));
	static IConsoleVariable* micInputGainCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.MicInputGain"));

	silenceDetectionReleaseCVar->Set(micNoiseGateThreshold);
	silenceDetectionThresholdCVar->Set(silenceDetectionThreshold);
	micInputGainCVar->Set(micInputGain);
}
bool AudioCaptureHandler::TryStartVoiceCapture()
{
	UE_LOGFMT(VoxtaLog, Log, "Starting voice capture.");

	if (!m_voiceCaptureDevice.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Error, "Unable to start capturing for sound wave, VoiceCapture pointer is invalid.");
		return false;
	}

	if (m_isCapturing)
	{
		UE_LOGFMT(VoxtaLog, Warning, "Voice capture is currently already capturing, ignoring new attempt to start.");
		return false;
	}

	m_decibels = DEFAULT_SILENCE_DECIBELS;
	// we let the thread notify us at 75% of the buffer, in case we encounter minor lag / delay,
	// so no audio data is lost. (within reason)
	m_voiceRunnerThread = MakeUnique<FVoiceRunnerThread>(this, (m_bufferMillisecondSize / 1000.f * 0.75f));
	if (!m_voiceRunnerThread.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Error, "VoiceRunnerThread has been destroyed.");
		return false;
	}
	

	if (!m_voiceCaptureDevice->Start())
	{
		UE_LOGFMT(VoxtaLog, Error, "Unable to start capturing for sound wave");
		return false;
	}

	m_voiceRunnerThread->Start();

	UE_LOGFMT(VoxtaLog, Log, "Successfully started voice capture & background thread");
	m_isCapturing = true;
	return true;
}

void AudioCaptureHandler::StopCapture()
{
	UE_LOGFMT(VoxtaLog, Log, "Stopping voice capture.");

	FScopeLock Lock(&m_captureGuard);
	m_decibels = DEFAULT_SILENCE_DECIBELS;
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

void AudioCaptureHandler::ShutDown(bool alsoDestroyCaptureDevice)
{
	StopCapture();
	FScopeLock Lock(&m_captureGuard);
	m_voiceRunnerThread = nullptr;
	if (alsoDestroyCaptureDevice)
	{
		m_voiceCaptureDevice = nullptr;
	}
}

bool AudioCaptureHandler::CaptureVoiceInternal(TArray<uint8>& voiceDataBuffer, float& decibels) const
{
	if (!m_voiceCaptureDevice.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Warning, "VoiceCapture device has been destroyed, can't capture any more data.");
		return false;
	}

	uint32 AvailableBytes = 0;

	if (m_voiceCaptureDevice->GetCaptureState(AvailableBytes) == EVoiceCaptureState::Ok)
	{
		if (AvailableBytes < 1)
		{
			return false;
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
		return true;
	}
	return false;
}

bool AudioCaptureHandler::IsInputSilent() const
{
	return !m_isCapturing ||
		FMath::IsNearlyEqual(m_decibels, DEFAULT_SILENCE_DECIBELS) ||
		(FDateTime::Now() - m_lastVoiceTimestamp).GetTotalSeconds() > 0.1f;
}

void AudioCaptureHandler::SendInternal(const TArray<uint8> rawData) const
{
	if (m_isTestMode)
	{
		return;
	}

	if (rawData.Num() > (m_bufferMillisecondSize * 32)) // 16 bits per sample
	{
		UE_LOGFMT(VoxtaLog, Warning, "VoiceCapture cannot send data, too big: {0}. Skipping.", rawData.Num());
		return;
	}

	if (TSharedPtr<AudioWebSocket> SharedSelf = m_webSocket.Pin())
	{
		SharedSelf->Send(rawData.GetData(), rawData.Num());
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "VoiceCapture instance was destroyed while trying to send data through the socket");
	}
}

void AudioCaptureHandler::CaptureAndSendVoiceData()
{
	FScopeLock Lock(&m_captureGuard);
	if (CaptureVoiceInternal(m_socketDataBuffer, m_decibels))
	{
		m_lastVoiceTimestamp = FDateTime::Now();
	}

	if (m_socketDataBuffer.Num() > 0)
	{
		SendInternal(m_socketDataBuffer);

		m_socketDataBuffer.Reset();
	}
}

float AudioCaptureHandler::GetDecibels() const
{
	FScopeLock Lock(&m_captureGuard);
	return IsInputSilent() ? DEFAULT_SILENCE_DECIBELS : m_decibels;
}

const FString& AudioCaptureHandler::GetDeviceName() const
{
	return m_deviceName;
}

void AudioCaptureHandler::SetIsTestMode(bool isTestMode)
{
	m_isTestMode = isTestMode;
}

float AudioCaptureHandler::AnalyseDecibels(const TArray<uint8>& voiceInputData, uint32 dataSize) const
{
	int16 sample;
	float sumSquared = 0.f;

	for (uint32 i = 0; i < dataSize / 2; ++i)
	{
		sample = (voiceInputData[i * 2 + 1] << 8) | voiceInputData[i * 2];
		sumSquared += static_cast<float>(sample) * static_cast<float>(sample);
	}

	float rms = FMath::Sqrt(sumSquared / (dataSize / 2.f));
	float decibels = 20.0f * FMath::LogX(10.f, FMath::Max(rms, 1.f) / 32768.f);
	return FMath::Max(decibels, DEFAULT_SILENCE_DECIBELS);
}