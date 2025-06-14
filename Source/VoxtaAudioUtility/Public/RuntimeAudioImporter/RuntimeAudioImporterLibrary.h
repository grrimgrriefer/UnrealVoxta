// Georgy Treshchev 2024.

#pragma once

#include "CoreMinimal.h"
#include "AudioStructs.h"

class UImportedSoundWave;

/**
 * Utility class for importing, decoding, resampling, and mixing audio data at runtime.
 * Provides static methods to import audio from memory buffers, decode audio data, and process audio channels and sample rates.
 */
class VOXTAAUDIOUTILITY_API URuntimeAudioImporterLibrary
{
public:
	/**
	 * Imports audio data from a memory buffer asynchronously.
	 * Bulk processing occurs on a background thread, but the callback executes on the game thread.
	 *
	 * @param buffer The audio data buffer to import.
	 * @param callback Callback function to invoke with the resulting `UImportedSoundWave*` on the game thread.
	 */
	static void ImportAudioFromBuffer(TArray64<uint8> buffer, TFunction<void(UImportedSoundWave*)> callback);

	/**
	 * Resamples and mixes the channels of decoded audio data.
	 *
	 * @param DecodedAudioInfo The decoded audio information to process.
	 * @param NewSampleRate The desired sample rate.
	 * @param NewNumOfChannels The desired number of channels.
	 * 
	 * @return True if successful, false otherwise.
	 */
	static bool ResampleAndMixChannelsInDecodedInfo(FDecodedAudioStruct& DecodedAudioInfo, uint32 NewSampleRate, uint32 NewNumOfChannels);

private:
	/**
	 * Imports audio from decoded audio information on the game thread.
	 *
	 * @param DecodedAudioInfo The decoded audio information. Ownership is transferred via move semantics; must be passed as an rvalue (`MoveTemp`).
	 * @param callback Callback function to invoke with the resulting `UImportedSoundWave*` on the game thread.
	 */

	static void ImportAudioFromDecodedInfo(FDecodedAudioStruct&& DecodedAudioInfo, TFunction<void(UImportedSoundWave*)> callback);

	/**
	 * Decodes encoded audio data into a decoded audio structure.
	 *
	 * @param EncodedAudioInfo The encoded audio information.
	 * @param DecodedAudioInfo The output decoded audio information.
	 * 
	 * @return True if decoding was successful, false otherwise.
	 */
	static bool DecodeAudioData(FEncodedAudioStruct&& EncodedAudioInfo, FDecodedAudioStruct& DecodedAudioInfo);
};
