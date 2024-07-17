// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AudioCaptureHandler.h"
#include "VoiceRunnerThread.h"

void AudioCaptureHandler::RegisterSocket(TSharedPtr<AudioWebSocket> socket)
{
	m_socket = socket;
}

bool AudioCaptureHandler::TryInitialize()
{
	if (VoiceCapture)
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
		AudioCapture.GetCaptureDeviceInfo(OutInfo, INDEX_NONE);
		m_deviceName = OutInfo.DeviceName;
		UE_LOG(AudioLog, Log, TEXT("Using microphone device %s"), *m_deviceName);

		VoiceCapture = VoiceModule.CreateVoiceCapture(m_deviceName, 16000, 1);
	}
	if (VoiceCapture.IsValid())
	{
		UE_LOG(AudioLog, Log, TEXT("Created voice caputre"));
		return true;
	}
	else
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to create the voice caputre"));
		return false;
	}
}

bool AudioCaptureHandler::StartCapture()
{
	m_voiceRunnerThread = MakeUnique<FVoiceRunnerThread>(this, 0.15f);
	if (!m_voiceRunnerThread.IsValid())
	{
		UE_LOG(LogVoice, Error, TEXT("m_voiceRunnerThread is null! Func: %s; Line: %s"), *FString(__FUNCTION__), *FString::FromInt(__LINE__));
		return false;
	}

	if (!VoiceCapture->Start())
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to start capturing for sound wave"));
		return false;
	}

	m_voiceRunnerThread->Start();

	UE_LOG(AudioLog, Log, TEXT("We streaming audio boys"));
	m_isCapturing = true;
	return true;
}

void AudioCaptureHandler::StopCapture()
{
	if (m_voiceRunnerThread.IsValid())
	{
		m_voiceRunnerThread->Stop();
	}

	ReplicatedBuffer.Reset();
	m_isCapturing = false;
	VoiceCapture->Stop();
}

void AudioCaptureHandler::ShutDown()
{
	StopCapture();
	m_voiceRunnerThread = nullptr;
}

void AudioCaptureHandler::CaptureVoice()
{
	if (!VoiceCapture.IsValid())
	{
		return;
	}

	uint32 AvailableBytes = 0;
	auto CaptureState = VoiceCapture->GetCaptureState(AvailableBytes);

	if (AvailableBytes < 1)
	{
		return;
	}

	VoiceCaptureBuffer.Reset();

	if (CaptureState == EVoiceCaptureState::Ok)
	{
		uint32 VoiceCaptureReadBytes = 0;

		VoiceCaptureBuffer.SetNumUninitialized(AvailableBytes);
		VoiceCapture->GetVoiceData(
			VoiceCaptureBuffer.GetData(),
			AvailableBytes,
			VoiceCaptureReadBytes
		);

		ReplicatedBuffer.Append(VoiceCaptureBuffer);
	}
}

void AudioCaptureHandler::Send(const TArray<uint8> InData)
{
	UE_LOG(LogTemp, Log, TEXT("Client: Sending %d"), InData[0]);
	if (InData.Num() > (200 * 32))
	{
		// TODO: Fix this, maybe, idk
		UE_LOG(LogTemp, Error, TEXT("Client: cannot send data, too big: %d"), InData.Num());
		return;
	}
	m_socket->Send(InData.GetData(), InData.Num());
}

void AudioCaptureHandler::CaptureAndSendVoiceData_Implementation()
{
	CaptureVoice();

	if (ReplicatedBuffer.Num() > 0)
	{
		Send(ReplicatedBuffer);

		ReplicatedBuffer.Reset();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No data to send"));
	}
}

float AudioCaptureHandler::GetNormalizedAmplitude() const
{
	if (VoiceCapture.IsValid())
	{
		return FMath::Clamp(VoiceCapture->GetCurrentAmplitude() / 20.f, 0.f, 1.f);
	}
	return 0.f;
}

FString AudioCaptureHandler::GetDeviceName() const
{
	return m_deviceName;
}