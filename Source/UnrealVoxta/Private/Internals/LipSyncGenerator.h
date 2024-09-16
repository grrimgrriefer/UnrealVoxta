// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

#if WITH_OVRLIPSYNC
class ULipSyncDataOVR;
#endif
class ULipSyncDataA2F;
class Audio2FaceRESTHandler;

/**
 * LipSyncGenerator
 * Internal helper class with static functions
 * Downloads the data from the VoxtaServer REST api, converting it into a SoundWave, and generating lipsync data.
 *
 * Note: The private API hooks into callbacks from background-threads, use care when changing the implementation.
 */
class LipSyncGenerator
{
public:
#if WITH_OVRLIPSYNC
	/**
	 * Generate a UOVRLipSyncFrameSequence in a background thread and attach it to the ULipSyncDataOVR instance.
	 *
	 * @param rawAudioData The raw audiodata in bytes.
	 * @param callback The callback that will be triggered when the OVR lipsync data has been created & pushed
	 * back on the gamethread.
	 */
	static void GenerateOVRLipSyncData(const TArray<uint8>& rawAudioData, TFunction<void(ULipSyncDataOVR*)> callback);
#endif

	/**
	 * Generate the A2F curves based in a background thread and attach it to the ULipSyncDataOVR instance.
	 *
	 * @param rawAudioData The raw audiodata in bytes.
	 * @param A2FRestHandler A pointer to the A2F REST APi handler.
	 * @param callback The callback that will be triggered when the A2F curves have been created and imported
	 * back into the gamethread.
	 */
	static void GenerateA2FLipSyncData(const TArray<uint8>& rawAudioData, Audio2FaceRESTHandler* A2FRestHandler,
		TFunction<void(ULipSyncDataA2F*)> callback);
};
