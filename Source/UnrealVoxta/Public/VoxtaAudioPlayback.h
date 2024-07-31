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
/// Public-facing class used to playback audio for a specific AI character.
/// Receives the URLs used to download & import & play the audio.
/// </summary>
UCLASS(HideCategories = (Mobility, Rendering, LOD), Blueprintable, ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class UNREALVOXTA_API UVoxtaAudioPlayback : public UActorComponent
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaMessageAudioPlaybackCompleted, const FString&, messageId);

	/// <summary>
	/// Event fired when the UAudioComponent reports that the audio has finished playing.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxtaMessageAudioPlaybackCompleted VoxtaMessageAudioPlaybackEvent;

	/// <summary>
	/// Adds the Audiocomponent as sub-object, enables ticks and also initializes the importerLibraryUtility.
	/// </summary>
	UVoxtaAudioPlayback();

	/// <summary>
	/// Configures the component so it only will playback messages for the specific character.
	/// </summary>
	/// <param name="voxtaClient">A pointer to the client, used to subscribe & unsubscribe to the messageEvents.</param>
	/// <param name="characterId">The ID of the character for which this component will be playing the audio.</param>
	void InitializeAudioPlayback(UVoxtaClient* voxtaClient, FStringView characterId);

	/// <summary>
	/// Returns true if this component is busy playing audio.
	/// </summary>
	/// <returns>Returns true if this component is busy playing audio.</returns>
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

	/// <summary>
	/// The main entrypoint, hooked into the VoxtaClient and will trigger the download & playback of the audio.
	/// </summary>
	/// <param name="sender">The characterID, will skip any messages not assigned to this character.</param>
	/// <param name="message">The message of which the URLs will be used to fetch the wav audio data.</param>
	UFUNCTION()
	void PlaybackMessage(const FCharDataBase& sender, const FChatMessage& message);

	/// <summary>
	/// Triggers async GET http requests for every url in the m_orderedUrls array.
	/// </summary>
	void DownloadDataAsync();

	/// <summary>
	/// Adds the host address and port to the partial uri that's baked into the ChatMessage.
	/// </summary>
	/// <param name="message">The chatMessage for which the urls will be constructed.</param>
	void GenerateFullUrls(const FChatMessage& message);

	/// <summary>
	/// If the next audioclip is available, begins playing it.
	/// Note: Audio is downloaded & imported in parallel, but obviously we want to playback in correct order.
	/// </summary>
	void TryPlayNextAudio();

	/// <summary>
	/// Triggered by the IHttpRequest when it's finished downloading the audio data.
	/// Will use the audioImporter to convert the wav file into a Soundwave that UE can play.
	/// </summary>
	/// <param name="request">Pointer to the original request</param>
	/// <param name="response">Pointer to the response</param>
	/// <param name="bWasSuccessful">True if the request was successful</param>
	void OnDownloadComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful);

	/// <summary>
	/// Triggered by the UAudioComponent.
	/// Will try to play the next audio component if available.
	/// </summary>
	UFUNCTION()
	void OnAudioFinished();

	/// <summary>
	/// Triggered by the AudioImporterLibrary.
	/// Adds the soundwave to the root to prevent cleanup, also triggers playback of the next clip
	/// if available. (as this one may or may not be the next audio we're waiting on)
	/// </summary>
	/// <param name="identifier">The identifier used to ensure audio is played in the correct order.</param>
	/// <param name="soundWave">The soundwave that is playable by the UAudioComponent</param>
	UFUNCTION()
	void AudioImportCompleted(FString identifier, UImportedSoundWave* soundWave);
};
