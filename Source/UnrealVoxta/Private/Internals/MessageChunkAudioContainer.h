// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncType.h"
#include "MessageChunkState.h"

class UImportedSoundWave;
class Audio2FaceRESTHandler;
class ILipSyncBaseData;

/**
 * MessageChunkAudioContainer
 * Internal utility class that encapsulates the download, decoding, and lipsync data generation for a single
 * audio chunk (voiceline) of an AI character. Handles async download from the VoxtaServer REST API,
 * conversion to a playable sound wave, and optional lipsync data generation (A2F, OVR, or custom).
 *
 * The object manages its own state machine and notifies a callback on state transitions.
 * Not a UObject; must be managed via shared pointers.
 */
class MessageChunkAudioContainer : public TSharedFromThis<MessageChunkAudioContainer>
{
#pragma region public API
public:
	/**
	 * Construct a new MessageChunkAudioContainer for a specific audio chunk.
	 *
	 * @param fullUrl The full URL to download the audio data from (VoxtaServer REST API).
	 * @param lipSyncType The type of lipsync data to generate for this chunk.
	 * @param A2FRestHandler Weak pointer to the A2F REST handler (required for A2F lipsync).
	 * @param callback Callback to invoke on state transitions.
	 * @param id Index of this chunk in the parent VoxtaAudioPlayback's chunk list.
	 *
	 * TODO: avoid requiring the A2FRestHandler injection, I kinda wanna move it to main subsystem but idk yet.
	 */
	MessageChunkAudioContainer(const FString& fullUrl,
		LipSyncType lipSyncType,
		TWeakPtr<Audio2FaceRESTHandler> A2FRestHandler,
		TFunction<void(const MessageChunkAudioContainer* newState)> callback,
		int id);

	virtual ~MessageChunkAudioContainer() = default;

	/**
	 * Advance the internal state machine to the next processing step (download, decode, lipsync, etc).
	 * Should be called when the previous step is complete.
	 */
	void Continue();

	/**
	 * Clean up all dynamically created objects and data, and mark this chunk as cleaned up.
	 * After this call, the chunk cannot be used again.
	 */
	void CleanupData();

	/**
	 * Get a reference to the raw audio data bytes for this chunk.
	 *
	 * @return Immutable reference to the raw audio data.
	 *
	 * Note: Main use-case is for custom lipsync, where blueprints could want access to the bytes to do whatever custom logic.
	 */
	const TArray<uint8>& GetRawAudioData() const;

	/**
	 * Get an immutable pointer to the lipsync data object for this chunk, cast to the requested type.
	 * @tparam T Must derive from ILipSyncBaseData.
	 * @return Pointer to the lipsync data object, or nullptr if not available.
	 */
	template <typename T, typename = std::enable_if_t<std::is_base_of_v<ILipSyncBaseData, T>>>
	const T* GetLipSyncData() const
	{
		return StaticCast<const T*>(m_lipSyncData);
	}

	/** @return The current state of this instance. */
	MessageChunkState GetCurrentState() const;

	/**
	 * @return The pointer to the generated SoundWave, tied to this instance.
	 *
	 * Note: UE requires a non-const pointer to play, which is why this doesn't return an immutable pointer.
	 */
	UImportedSoundWave* GetSoundWave() const;

#pragma endregion

#pragma region data
public:
	/** The index of this sound asset in the collection of the VoxtaAudioPlayback. */
	const int INDEX;

	/** The type of lipsync that this voiceline instance will support with its data. */
	const LipSyncType LIP_SYNC_TYPE;

private:
	const FString FULL_DOWNLOAD_URL;
	const TFunction<void(const MessageChunkAudioContainer* chunk)> ON_STATE_CHANGED;

	TArray<uint8> m_rawAudioData;
	TWeakPtr<Audio2FaceRESTHandler> m_A2FRestHandler = nullptr;
	MessageChunkState m_state = MessageChunkState::Idle;

	// UObjects added to root while this object is alive; as UPROPERTY doesn't work with normal classes
	UImportedSoundWave* m_soundWave = nullptr;
	ILipSyncBaseData* m_lipSyncData = nullptr;
#pragma endregion

#pragma region private API
private:
	/** Download the audio data into memory from the VoxtaServer REST api. */
	void DownloadData();

	/** Convert the imported raw audiodata into a UImportedSoundWave that can be played. */
	void ProcessAudioData();

	/**
	 * Generate lipsync data based on the raw audiodata, and populate the m_lipSyncData with the result
	 *
	 * Note: This only supports A2F and OVR at the moment.
	 */
	void GenerateLipSync();

	/**
	 * Update the internal state to keep track where this instance is with processing the data for the voiceline.
	 *
	 * @param newState The new state that this instance should consider itself in. (will be broadcasted after setting internally)
	 */
	void UpdateState(MessageChunkState newState);
#pragma endregion
};
