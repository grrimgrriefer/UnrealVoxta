// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "VoxtaAudioPlaybackBase.h"
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
class UNREALVOXTA_API UVoxtaAudioPlayback : public UAudioComponent, public IVoxtaAudioPlaybackBase
{
	GENERATED_BODY()

#pragma region internal helper classes
private:
	/** Internal helper enum to keep track of what we are doing at the moment. */
	enum class InternalState : uint8
	{
		Idle,
		Playing,
		Done
	};
#pragma endregion

public:
#pragma region delegate declarations
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaMessageAudioPlaybackCompleted, const FString&, messageId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVoxtaMessageAudioChunkReadyForCustomPlayback, const TArray<uint8>&, rawBytes, const USoundWaveProcedural*, processedSoundWave, const FGuid&, audioChunkGuid);

	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaMessageAudioPlaybackCompletedNative, const FString&);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FVoxtaMessageAudioChunkReadyForCustomPlaybackNative, const TArray<uint8>&, const USoundWaveProcedural*, const FGuid&);
#pragma endregion

#pragma region events
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
	void InitializeAudioPlayback(const FString& characterId);

	/**
	 * Notify that the Audio is done with playback. Due to the unpredictable nature of the blueprint, we rely on
	 * a manual call for this, to avoid any confusion or false positives.
	 *
	 * @param guid The guid that was passed in the VoxtaMessageAudioChunkReadyForCustomPlaybackEvent.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void MarkCustomPlaybackComplete(const FGuid& guid);
#pragma endregion

#pragma region UActorComponent overrides
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
#pragma endregion

#pragma region IVoxtaAudioPlaybackBase overrides
public:
	virtual void GetA2FCurveWeights(TArray<float>& targetArrayRef) override;
#pragma endregion

#pragma region data
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true"))
	LipSyncType m_lipSyncType;

	UPROPERTY()
	UActorComponent* m_ovrLipSync;

	UPROPERTY()
	UAudio2FacePlaybackHandler* m_audio2FacePlaybackHandler;

	UVoxtaClient* m_clientReference;
	FString m_characterId;
	FString m_messageId;
	TArray<TSharedPtr<MessageChunkAudioContainer>> m_orderedAudio;

	FDelegateHandle m_charMessageAddedHandle;
	FDelegateHandle m_playbackFinishedHandle;

	FString m_hostAddress;
	int m_hostPort;
	InternalState m_internalState;
	int m_currentAudioClip = 0;
#pragma endregion

#pragma region private API
	/**
	 * The main entrypoint, hooked into the VoxtaClient and will trigger the download & playback of the audio.
	 *
	 * @param sender The characterID, will skip any messages not assigned to this character.
	 * @param message The message of which the URLs will be used to fetch the wav audio data.
	 */
	UFUNCTION()
	void PlaybackMessage(const FCharDataBase& sender, const FChatMessage& message);

	/** Try to begin playing the next audioclip, if it is available */
	void TryPlayCurrentAudioChunk();

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
