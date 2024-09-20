// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "AbstractA2FWeightProvider.h"
#include "LipSyncType.h"
#include "VoxtaAudioPlayback.generated.h"

class MessageChunkAudioContainer;
class UActorComponent;
class UAudio2FacePlaybackHandler;
class UVoxtaClient;

/**
 * UVoxtaAudioPlayback
 * Public-facing component, can be placed on an Actor to playback audio for a specific AI Character.
 * Internally handles downloading the audio from the VoxtaServer, importing & parsing it to generating lipsync data.
 * Also handles the automatic playback unless 'custom lipsync' is enabled.
 */
UCLASS(HideCategories = (Mobility, Rendering, LOD), ClassGroup = Voxta, meta = (BlueprintSpawnableComponent))
class UNREALVOXTA_API UVoxtaAudioPlayback : public UAudioComponent, public IA2FWeightProvider
{
	GENERATED_BODY()

#pragma region delegate declarations
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaMessageAudioPlaybackCompleted, const FString&, messageId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVoxtaMessageAudioChunkReadyForCustomPlayback, const TArray<uint8>&, rawBytes, const USoundWaveProcedural*, processedSoundWave, const FGuid&, audioChunkGuid);

	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaMessageAudioPlaybackCompletedNative, const FString&);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FVoxtaMessageAudioChunkReadyForCustomPlaybackNative, const TArray<uint8>&, const USoundWaveProcedural*, const FGuid&);
#pragma endregion

#pragma region events
public:
	/** Event fired when the UAudioComponent reports that the audio has finished playing. */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaMessageAudioPlaybackCompleted VoxtaMessageAudioPlaybackFinishedEvent;
	FVoxtaMessageAudioPlaybackCompletedNative VoxtaMessageAudioPlaybackFinishedEventNative;

	/**
	 * Event fired when a chunk of audio data is ready and is marked for 'custom' lipsync.
	 * This means we have imported and processed to audio into a SoundWave. But lipsync and playback must be handled by your own logic.
	 *
	 * Note: After the playback is complete, please call MarkCustomPlaybackComplete with the guid provided.
	 *
	 * Warning: The entire message can be multiple sequential audio chunks. Make sure to handle this properly.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaMessageAudioChunkReadyForCustomPlayback VoxtaMessageAudioChunkReadyForCustomPlaybackEvent;
	FVoxtaMessageAudioChunkReadyForCustomPlaybackNative VoxtaMessageAudioChunkReadyForCustomPlaybackEventNative;
#pragma endregion

#pragma region public API
public:
	/**
	 * Configures the component so it only will playback messages for the specific character.
	 *
	 * @param characterId The ID of the character for which this component will be playing the audio.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void Initialize(const FString& characterId);

	/**
	 * Notify that the Audio is done with playback. Due to the unpredictable nature of the blueprint, we rely on
	 * a manual call for this, to avoid any confusion or false positives.
	 *
	 * @param guid The guid that was passed in the VoxtaMessageAudioChunkReadyForCustomPlaybackEvent.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void MarkCustomPlaybackComplete(const FGuid& guid);
#pragma endregion

#pragma region IA2FWeightProvider overrides
public:
	/**
	 * Retrieve the A2F curveWeights for the upcoming update tick.
	 *
	 * @param targetArrayRef The array that will be used to fill the new curves. Old values will be overwritten.
	 */
	virtual void GetA2FCurveWeightsPreUpdate(TArray<float>& targetArrayRef) override;
#pragma endregion

#pragma region UActorComponent overrides
protected:
	/**
	 * Begins Play for the component.
	 * Called when the owning Actor begins play or when the component is created if the Actor has already begun play.
	 * Actor BeginPlay normally happens right after PostInitializeComponents but can be delayed for networked or child actors.
	 * Requires component to be registered and initialized.
	 */
	virtual void BeginPlay() override;

	/**
	 * Ends gameplay for this component.
	 * Called from AActor::EndPlay only if bHasBegunPlay is true
	 */
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
#pragma endregion

#pragma region private helper classes
private:
	/** Internal helper class, easier to keep track of what's going on, as well as user-friendly logging. */
	enum class AudioPlaybackInternalState : uint8
	{
		Idle,
		Playing,
		Done
	};
#pragma endregion

#pragma region data
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxta",
		meta = (AllowPrivateAccess = "true", DisplayName = "Lipsync Type"))
	LipSyncType m_lipSyncType;

	// TODO: use interface so we don't have to cast to cast to UAudio2FacePlaybackHandler or
	// UOVRLipSyncPlaybackActorComponent everytime
	UPROPERTY()
	UObject* m_lipSyncHandler;

	UVoxtaClient* m_clientReference;
	FString m_characterId;
	FString m_currentlyPlayingMessageId;
	TArray<TSharedPtr<MessageChunkAudioContainer>> m_orderedAudio;

	FDelegateHandle m_charMessageAddedHandle;
	FDelegateHandle m_playbackFinishedHandle;

	FString m_hostAddress;
	int m_hostPort;
	AudioPlaybackInternalState m_internalState;
	int m_currentAudioClipIndex = 0;
#pragma endregion

#pragma region private API
private:
	/**
	 * The main entrypoint, hooked into the VoxtaClient and will trigger the download & playback of the audio.
	 *
	 * @param sender The characterID, will skip any messages not assigned to this character.
	 * @param message The message of which the URLs will be used to fetch the wav audio data.
	 */
	UFUNCTION()
	void PlaybackMessage(const FBaseCharData& sender, const FChatMessage& message);

	/** Begin playing the audioclip on the currently marked index, if it is available */
	void PlayCurrentAudioChunkIfAvailable();

	/**
	 * Triggered by the UAudioComponent, will trigger playback of the next chunk if it is present.
	 *
	 * Note: This does nothing when using Custom LipSync, as then we're waitin for 'MarkCustomPlaybackComplete'
	 *
	 * @param component A pointer to the component that has finished it's playback
	 * (should always be a pointer to itself).
	 */
	UFUNCTION()
	void OnAudioPlaybackFinished(UAudioComponent* component);

	/**
	 * Keeps track of which index is currently being played
	 * and will trigger the finished event if we're done playing.
	 */
	void MarkAudioChunkPlaybackCompleteInternal();

	/**
	 * Triggers the next state for the container as well as triggering other parallel jobs if applicable.
	 *
	 * @param finishedChunk An immutable pointer to the instance that had its state changed.
	 */
	void OnChunkStateChange(const MessageChunkAudioContainer* finishedChunk);

	/** Clean up the soundwaves correctly, so there's no memory leaks. */
	void Cleanup();
#pragma endregion
};
