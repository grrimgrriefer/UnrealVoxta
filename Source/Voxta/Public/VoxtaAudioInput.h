// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioUtility/Public/AudioWebSocket.h"

#include "VoxtaAudioInput.generated.h"

UCLASS(HideCategories = (Mobility, Rendering, LOD), Blueprintable, ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class VOXTA_API UVoxtaAudioInput : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxtaAudioInput();

	void RegisterEndpoint(const FString& serverIP, int serverPort);

	void StartStreaming();
	void StopStreaming();

private:
	//AudioCaptureDevice m_audioCaptureDevice;
	TSharedPtr<AudioWebSocket> m_audioWebSocket;

	//std::jthread m_startupThread;
	bool m_isStreaming = false;
	bool m_isStartingUp = false;
};
