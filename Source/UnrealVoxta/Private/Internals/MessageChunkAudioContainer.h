// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncType.h"
#include "MessageChunkState.h"

class USoundWaveProcedural;
class Audio2FaceRESTHandler;
class ILipSyncDataBase;

/**
 * MessageChunkAudioContainer
 * Internal class which encapsulates all datahandling for a single AI character voiceline.
 * Downloads the data from the VoxtaServer REST api, converting it into a SoundWave, and generating lipsync data.
 *
 * Note: The private API hooks into callbacks from background-threads, use care when changing the implementation.
 */
class MessageChunkAudioContainer : public TSharedFromThis<MessageChunkAudioContainer>
{
#pragma region public API
public:
	/**
	 * Initialize the instance with all the required data to prepare the audio & lipsync data in full isolation.
	 *
	 * @param fullUrl The full url to download the audio data. (Should be referring to the VoxtaSever REST-api)
	 * @param lipSyncType The type of lipsync generation that should be done for this voiceline.
	 * @param A2FRestHandler Pointer to access the A2F REST api.
	 * @param callback The function that will be triggered when this instance has completed a 'state' and is ready for the next.
	 * @param id The index of this sound asset in the collection of the VoxtaAudioPlayback.
	 *
	 * TODO: avoid requiring the A2FRestHandler injection, maybe statics?
	 */
	MessageChunkAudioContainer(const FString& fullUrl,
		LipSyncType lipSyncType,
		Audio2FaceRESTHandler* A2FRestHandler,
		TFunction<void(const MessageChunkAudioContainer* newState)> callback,
		int id);

	/** Trigger the next pre-processing state, assuming the instance is finished with what it was doing. */
	void Continue();

	/** Remove all dynamically created objects & data, also permanently marks the instance as cleaned up, so it can't be used anymore. */
	void CleanupData();

	/**
	 * @return An immutable reference to the raw audioData bytes.
	 *
	 * Note: Main use-case is for custom lipsync, where blueprints could want access to the bytes to do whatever custom logic.
	 */
	const TArray<uint8>& GetRawAudioData() const;

	/** @return an immutable pointer to the LipSync dataobject, this is guarenteed to implement the ILipSyncDataBase interface. */
	template<class T>
	const T* GetLipSyncData() const;

	/** @return An immutable reference to the current state of this instance. */
	const MessageChunkState& GetCurrentState() const;

	/** @return The pointer to the generated SoundWave, tied to this instance. */
	USoundWaveProcedural* GetSoundWave() const;

	/** The index of this sound asset in the collection of the VoxtaAudioPlayback. */
	const int INDEX;

	/** The type of lipsync that this voiceline instance will support with its data. */
	const LipSyncType LIP_SYNC_TYPE;
#pragma endregion

#pragma region data
private:
	const FString FULL_DOWNLOAD_URL;
	const TFunction<void(const MessageChunkAudioContainer* chunk)> ON_STATE_CHANGED;

	TArray<uint8> m_rawAudioData;
	Audio2FaceRESTHandler* m_A2FRestHandler;
	USoundWaveProcedural* m_soundWave;
	MessageChunkState m_state = MessageChunkState::Idle;
	ILipSyncDataBase* m_lipSyncData;
#pragma endregion

#pragma region private API
	/** Download the audio data into memory from the VoxtaServer REST api. */
	void DownloadData();

	/** Convert the imported raw audiodata into a UImportedSoundWave that can be played. */
	void ImportData();

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
