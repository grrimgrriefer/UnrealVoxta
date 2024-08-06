// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "VoxtaClient.h"
#include "Components/AudioComponent.h"
#include "AudioUtility/Public/RuntimeAudioImporter/RuntimeAudioImporterLibrary.h"
#include "MessageChunkAudioContainer.h"
#include "OVRLipSyncPlaybackActorComponent.h"
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
	TArray<MessageChunkAudioContainer> m_orderedAudio;

	UOVRLipSyncPlaybackActorComponent* m_ovrLipSync;

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
	/// If the next audioclip is available, begins playing it.
	/// Note: Audio is downloaded & imported in parallel, but obviously we want to playback in correct order.
	/// </summary>
	void TryPlayCurrentAudioChunk();

	/// <summary>
	/// Triggered by the UAudioComponent.
	/// Will try to play the next audio component if available.
	/// </summary>
	UFUNCTION()
	void OnAudioFinished();

	void OnChunkStateChange(const MessageChunkAudioContainer* finishedChunk);

	void Cleanup();
};
