// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "VoxtaClient.h"
#include "Components/AudioComponent.h"
#include "VoxtaAudioPlaybackBase.h"
#include "AudioUtility/Public/RuntimeAudioImporter/RuntimeAudioImporterLibrary.h"
#include "Internals/MessageChunkAudioContainer.h"
#if WITH_OVRLIPSYNC
#include "OVRLipSyncPlaybackActorComponent.h"
#endif
#include "LipSyncDataA2F.h"
#include "Audio2FacePlaybackHandler.h"
#include "VoxtaAudioPlayback.generated.h"

/// <summary>
/// Public-facing class used to playback audio for a specific AI character.
/// Receives the URLs used to download & import & play the audio.
/// </summary>
UCLASS(HideCategories = (Mobility, Rendering, LOD), ClassGroup = Voxta, meta = (BlueprintSpawnableComponent))
class UNREALVOXTA_API UVoxtaAudioPlayback : public UAudioComponent, public IVoxtaAudioPlaybackBase
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaMessageAudioPlaybackCompleted, const FString&, messageId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVoxtaMessageAudioChunkReadyForCustomPlayback, const TArray<uint8>&, rawBytes, const USoundWaveProcedural*, processedSoundWave, const FGuid&, audioChunkGuid);

	/// <summary>
	/// Event fired when the UAudioComponent reports that the audio has finished playing.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxtaMessageAudioPlaybackCompleted VoxtaMessageAudioPlaybackFinishedEvent;

	/// <summary>
	/// Event fired when a chunk of audio data is ready and is marked for 'custom' lipsync.
	/// We have only generated the audio file. Lipsync and audioplayback must be handled by 'custom' external logic.
	///
	/// NOTE: After the playback is complete, please call MarkCustomPlaybackComplete with the guid provided.
	/// WARNING: The entire message can be multiple sequential audio chunks. Make sure to handle this properly.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxtaMessageAudioChunkReadyForCustomPlayback VoxtaMessageAudioChunkReadyForCustomPlaybackEvent;

	/// <summary>
	/// Adds the Audiocomponent as sub-object, enables ticks and also initializes the importerLibraryUtility.
	/// </summary>
	UVoxtaAudioPlayback();

	/// <summary>
	/// Configures the component so it only will playback messages for the specific character.
	/// </summary>
	/// <param name="voxtaClient">A pointer to the client, used to subscribe & unsubscribe to the messageEvents.</param>
	/// <param name="characterId">The ID of the character for which this component will be playing the audio.</param>
	UFUNCTION(BlueprintCallable)
	void InitializeAudioPlayback(const FString& characterId);

	/// <summary>
	/// Notify that the Audio is done with playback. Due to the unpredictable nature of the blueprint, we rely on
	/// a manual call for this, to avoid any confusion or false positives.
	/// </summary>
	/// <param name="guid">The guid that was passed in the VoxtaMessageAudioChunkReadyForCustomPlaybackEvent</param>
	UFUNCTION(BlueprintCallable)
	void MarkCustomPlaybackComplete(const FGuid& guid);

	UFUNCTION(BlueprintCallable)
	void RegisterOVRLipSyncComponent();

	///~ Begin IVoxtaAudioPlaybackBase overrides.
public:
	virtual void GetA2FCurveWeights(TArray<float>& targetArrayRef) override;
	///~ End IVoxtaAudioPlaybackBase overrides.

	///~ Begin UActorComponent overrides.
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	///~ End UActorComponent overrides.

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	LipSyncType m_lipSyncType;

	UPROPERTY()
	UActorComponent* m_ovrLipSync;

	UPROPERTY()
	UAudio2FacePlaybackHandler* m_audio2FacePlaybackHandler;

private:
	enum class InternalState : uint8
	{
		Idle,
		Playing,
		Done
	};

	UVoxtaClient* m_clientReference;
	FString m_characterId;
	FString m_messageId;
	TArray<MessageChunkAudioContainer> m_orderedAudio;

	FString m_hostAddress;
	int m_hostPort;

	InternalState m_internalState;
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
	void OnAudioPlaybackFinished();

	void MarkAudioChunkPlaybackCompleteInternal();

	void OnChunkStateChange(const MessageChunkAudioContainer* finishedChunk);

	void Cleanup();
};
