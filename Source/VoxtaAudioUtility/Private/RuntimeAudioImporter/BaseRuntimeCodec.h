// Georgy Treshchev 2024.

#pragma once

#include "CoreMinimal.h"
#include "Features/IModularFeature.h"
#include "AudioStructs.h"

/**
 * Base runtime codec
 * To add a new codec, derive from this class and implement the necessary functions.
 * Then register the codec as a modular feature within your module, such as in the module's StartupModule function.
 * See FRuntimeAudioImporterModule::StartupModule() as a reference.
 */
class FBaseRuntimeCodec : public IModularFeature
{
public:
	FBaseRuntimeCodec() = default;
	virtual ~FBaseRuntimeCodec() = default;

	/**
	 * Check if the given audio data appears to be valid and optionally fix the format if needed.
	 * 
	 * @param AudioData The audio data buffer to check and fix.
	 * 
	 * @return True if the audio data is valid (and fixed if necessary), false otherwise.
	 */
	virtual bool CheckAndFixAudioFormat(FRuntimeBulkDataBuffer<uint8>& AudioData) PURE_VIRTUAL(FBaseRuntimeCodec::CheckAndFixAudioFormat, return false;)

	/**
	 * Retrieve audio header information from an encoded source.
	 * 
	 * @param EncodedData The encoded audio data.
	 * @param HeaderInfo Output parameter to receive the audio header information.
	 * 
	 * @return True if header information was successfully retrieved, false otherwise.
	 */
	virtual bool GetHeaderInfo(const FEncodedAudioStruct& EncodedData, FRuntimeAudioHeaderInfo& HeaderInfo) PURE_VIRTUAL(FBaseRuntimeCodec::GetHeaderInfo, return false;)

	/**
	 * Decode compressed audio data into PCM format.
	 * 
	 * 
	 * @param EncodedData The encoded audio data to decode.
	 * @param DecodedData Output parameter to receive the decoded audio data.
	 * 
	 * @return True if decoding was successful, false otherwise.
	 */
	virtual bool Decode(FEncodedAudioStruct EncodedData, FDecodedAudioStruct& DecodedData) PURE_VIRTUAL(FBaseRuntimeCodec::Decode, return false;)

	/**
	 * Retrieve the format applicable to this codec.
	 * 
	 * @return The audio format supported by this codec.
	 */
	virtual ERuntimeAudioFormat GetAudioFormat() const PURE_VIRTUAL(FBaseRuntimeCodec::GetAudioFormat, return ERuntimeAudioFormat::Invalid;)
};
