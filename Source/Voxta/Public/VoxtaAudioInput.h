// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioUtility/Public/AudioWebSocket.h"
#include "AudioUtility/Public/AudioCaptureHandler.h"
//#include "AudioUtility/Public/RuntimeAudioImporter/CapturableSoundWave.h"

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
	UVoxtaAudioInput();

	void InitializeSocket(const FString& serverIP, int serverPort);
	void CloseSocket();

	void StartStreaming();
	void StopStreaming();

private:
	//UCapturableSoundWave* m_audioCaptureDevice;
	AudioCaptureHandler m_audioCaptureDevice;
	TSharedPtr<AudioWebSocket> m_audioWebSocket;
	MicrophoneSocketState m_connectionState;

	void OnSocketConnected();
	void OnSocketConnectionError(const FString& error);
	void OnSocketClosed(int StatusCode, const FString& Reason, bool bWasClean);

	//	UFUNCTION()
	//	void OnAudioDataAdded(const TArray<uint8>& PopulatedAudioData);
};
