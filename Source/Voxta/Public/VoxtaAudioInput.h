// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioUtility/Public/AudioWebSocket.h"
#include "AudioUtility/Public/AudioCaptureHandler.h"
#include "VoxtaAudioInput.generated.h"

UENUM(BlueprintType)
enum class MicrophoneSocketState : uint8
{
	NotConnected			UMETA(DisplayName = "NotConnected"),
	Initializing			UMETA(DisplayName = "Initializing"),
	Ready					UMETA(DisplayName = "Ready"),
	InUse					UMETA(DisplayName = "Ready"),
	Closed					UMETA(DisplayName = "Closed")
};

UCLASS(HideCategories = (Mobility, Rendering, LOD), Blueprintable, ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class VOXTA_API UVoxtaAudioInput : public UActorComponent
{
	GENERATED_BODY()

public:
	void InitializeSocket(const FString& serverIP,
		int serverPort,
		int bufferMs = 200,
		int sampleRate = 16000,
		int inputChannels = 1);
	void CloseSocket();

	void StartStreaming();
	void StopStreaming();

	bool IsRecording() const;
	float GetNormalizedAmplitude() const;
	FString GetInputDeviceName() const;

private:
	int m_bufferMs;
	int m_sampleRate;
	int m_inputChannels;

	AudioCaptureHandler m_audioCaptureDevice;
	TSharedPtr<AudioWebSocket> m_audioWebSocket;
	MicrophoneSocketState m_connectionState;

	void OnSocketConnected();
	void OnSocketConnectionError(const FString& error);
	void OnSocketClosed(int StatusCode, const FString& Reason, bool bWasClean);

	///~ Begin UActorComponent overrides.
protected:
	virtual void BeginDestroy() override;
	///~ End UActorComponent overrides.
};
