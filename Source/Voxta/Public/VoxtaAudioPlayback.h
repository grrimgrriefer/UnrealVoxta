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

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaMessageAudioPlaybackCompleted, const FString&, messageId);

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxtaMessageAudioPlaybackCompleted VoxtaMessageAudioPlaybackEvent;

	UVoxtaAudioPlayback();

	void InitializeAudioPlayback(UVoxtaClient* voxtaClient, FStringView characterId);

private:
	UVoxtaClient* m_clientReference;
	FString m_characterId;
	FString m_messageId;
	UAudioComponent* m_audioComponent;
	TArray<FString> m_orderedUrls;
	TMap<FString, USoundWaveProcedural*> m_audioData;
	UAudioImporter* audioImporter;

	FString m_hostAddress;
	FString m_hostPort;

	bool isPlaying;
	int currentAudioClip = 0;

	UFUNCTION()
	void PlaybackMessage(const FCharDataBase& sender, const FChatMessage& message);

	void ConvertRawAudioData(const FString& identifier, const TArray<uint8>& rawData);
	void DownloadDataAsync();
	void GenerateFullUrls(const FChatMessage& message);
	void TryPlayNextAudio();
	void OnDownloadComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful);

	UFUNCTION()
	void OnAudioFinished();

	UFUNCTION()
	void AudioImportCompleted(FString identifier, UImportedSoundWave* soundWave);

	///~ Begin UActorComponent overrides.
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	///~ End UActorComponent overrides.
};
