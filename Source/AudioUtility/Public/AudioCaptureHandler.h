// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "AudioWebSocket.h"
#include "AudioCaptureCore.h"
#include "Voice.h"
#include "RuntimeAudioImporter/AudioStructs.h"
#include "VoiceRunnerThread.h"

class AUDIOUTILITY_API AudioCaptureHandler
{
public:
	void RegisterSocket(TSharedPtr<AudioWebSocket> socket);
	bool TryInitialize();

	bool StartCapture();
	void StopCapture();
	void ShutDown();

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

	TUniquePtr<FVoiceRunnerThread> m_voiceRunnerThread;
	TSharedPtr<AudioWebSocket> m_socket;
	TSharedPtr<class IVoiceCapture> VoiceCapture;

	FString m_deviceName;

	bool m_isCapturing = false;

	friend class FVoiceRunnerThread;
};
