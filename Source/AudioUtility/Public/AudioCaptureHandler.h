// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "AudioWebSocket.h"
#include "AudioCaptureCore.h"
#include "Voice.h"
#include "RuntimeAudioImporter/AudioStructs.h"

class FVoiceRunnerThread;

class AUDIOUTILITY_API AudioCaptureHandler
{
public:
	void RegisterSocket(TSharedPtr<AudioWebSocket> socket);
	bool TryInitialize();

	bool StartCapture();
	void StopCapture();

	void CaptureAndSendVoiceData_Implementation();

	float GetNormalizedAmplitude() const;
	FString GetDeviceName() const;

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	TArray<uint8> VoiceCaptureBuffer;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	TArray<uint8> ReplicatedBuffer;

private:
	void CaptureVoice();
	void Send(const TArray<uint8> InData);

	TSharedPtr<FVoiceRunnerThread> m_voiceRunnerThread;
	TSharedPtr<IVoiceEncoder> VoiceEncoder;
	TSharedPtr<AudioWebSocket> m_socket;
	TSharedPtr<class IVoiceCapture> VoiceCapture;

	FString m_deviceName;

	bool m_isCapturing = false;

	friend class FVoiceRunnerThread;
};
