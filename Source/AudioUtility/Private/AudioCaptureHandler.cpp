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

		VoiceCapture = VoiceModule.CreateVoiceCapture("", 16000, 1);
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
	m_voiceRunnerThread = MakeShared<FVoiceRunnerThread>(this, 0.15f);
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

	m_isCapturing = false;
	VoiceCapture->Stop();
}

void AudioCaptureHandler::CaptureVoice()
{
	if (!VoiceCapture.IsValid()) { return; }

	uint32 AvailableBytes = 0;
	auto CaptureState = VoiceCapture->GetCaptureState(AvailableBytes);

	VoiceCaptureBuffer.Reset();

	if (CaptureState == EVoiceCaptureState::Ok && AvailableBytes > 0)
	{
		short VoiceCaptureSample = 0;
		uint32 VoiceCaptureReadBytes = 0;
		float VoiceCaptureTotalSquared = 0;

		VoiceCaptureBuffer.SetNumUninitialized(AvailableBytes);
		VoiceCapture->GetVoiceData(
			VoiceCaptureBuffer.GetData(),
			AvailableBytes,
			VoiceCaptureReadBytes
		);

		for (uint32 i = 0; i < (VoiceCaptureReadBytes / 2); i++)
		{
			VoiceCaptureSample = (VoiceCaptureBuffer[i * 2 + 1] << 8) | VoiceCaptureBuffer[i * 2];
			VoiceCaptureTotalSquared += float(VoiceCaptureSample) * float(VoiceCaptureSample);
		}

		float VoiceCaptureMeanSquare = 2 * (VoiceCaptureTotalSquared / VoiceCaptureBuffer.Num());
		float VoiceCaptureRms = FMath::Sqrt(VoiceCaptureMeanSquare);
		float VoiceCaptureFinalVolume = VoiceCaptureRms / 32768.0 * 200.f;

		VoiceCaptureVolume = VoiceCaptureFinalVolume;

		auto TempEncodeBuffer = TArray<uint8>();
		auto CompressedSize = UVOIPStatics::GetMaxCompressedVoiceDataSize();
		TempEncodeBuffer.SetNumUninitialized(CompressedSize);
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