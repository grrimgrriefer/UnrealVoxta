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
#include "AudioUtility/Public/RuntimeAudioImporter/RuntimeAudioImporterLibrary.h"
#include "VoxtaAudioPlayback.generated.h"

/// <summary>
/// Public-facing class used to playback audio for a specific character.
/// Receives the URLs used to download & import & play the audio.
/// Note: The VoxtaClient will wait VoxtaMessageAudioPlaybackEvent to continue.
/// </summary>
UCLASS(HideCategories = (Mobility, Rendering, LOD), Blueprintable, ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class VOXTA_API UVoxtaAudioPlayback : public UActorComponent
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaMessageAudioPlaybackCompleted, const FString&, messageId);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxtaMessageAudioPlaybackCompleted VoxtaMessageAudioPlaybackEvent;

	UVoxtaAudioPlayback();

	void InitializeAudioPlayback(UVoxtaClient* voxtaClient, FStringView characterId);
	bool IsPlaying() const;

	///~ Begin UActorComponent overrides.
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	///~ End UActorComponent overrides.

private:
	UVoxtaClient* m_clientReference;
	FString m_characterId;
	FString m_messageId;
	UAudioComponent* m_audioComponent;
	TArray<FString> m_orderedUrls;
	TMap<FString, USoundWaveProcedural*> m_audioData;

	UPROPERTY(Transient)
	URuntimeAudioImporterLibrary* audioImporter;

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
};
