// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "VoxtaClient.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Sound/SoundWaveProcedural.h"

#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioUtility/Public/AudioImporter.h"

#include "VoxtaAudioPlayback.generated.h"

UCLASS(HideCategories = (Mobility, Rendering, LOD), Blueprintable, ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class VOXTA_API UVoxtaAudioPlayback : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxtaAudioPlayback();

	void InitializeAudioPlayback(UVoxtaClient* voxtaClient, FStringView characterId);

private:
	FString m_characterId;
	UAudioComponent* m_audioComponent;
	TArray<FString> m_orderedUrls;
	TMap<FString, USoundWaveProcedural> m_audioData;
	AudioImporter audioImporter;

	USoundCue* propellerAudioCue;

	FString m_hostAddress;
	FString m_hostPort;

	UFUNCTION()
	void PlaybackMessage(const FCharDataBase& sender, const FChatMessage& message);

	USoundWaveProcedural ConvertRawAudioData(const TArray<uint8>& rawData);
	void DownloadDataAsync();
	void GenerateFullUrls(const FChatMessage& message);
	void TriggerPlayback();
	void OnDownloadComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful);
};
