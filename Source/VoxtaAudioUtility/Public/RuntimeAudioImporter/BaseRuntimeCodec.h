// Georgy Treshchev 2024.

#pragma once

#include "CoreMinimal.h"
#include "Features/IModularFeature.h"
#include "AudioStructs.h"

/**
 * Base runtime codec
 * To add a new codec, derive from this class and implement the necessary functions
 * Then register the codec as a modular feature within your module, such as in the module's StartupModule function
 * See FRuntimeAudioImporterModule::StartupModule() as a reference
 */
class FBaseRuntimeCodec : public IModularFeature
{
public:
	FBaseRuntimeCodec() = default;
	virtual ~FBaseRuntimeCodec() = default;

	/**
	 * Check if the given audio data appears to be valid
	 */
	virtual bool CheckAndFixAudioFormat(FRuntimeBulkDataBuffer<uint8>& AudioData) PURE_VIRTUAL(FBaseRuntimeCodec::CheckAudioFormat, return false;)

	/**
	 * Retrieve audio header information from an encoded source
	 */
	virtual bool GetHeaderInfo(const FEncodedAudioStruct& EncodedData, FRuntimeAudioHeaderInfo& HeaderInfo) PURE_VIRTUAL(FBaseRuntimeCodec::GetHeaderInfo, return false;)

	/**
 	 * Decode compressed audio data into PCM format
	 */
	virtual bool Decode(FEncodedAudioStruct EncodedData, FDecodedAudioStruct& DecodedData) PURE_VIRTUAL(FBaseRuntimeCodec::Decode, return false;)

	/**
	 * Retrieve the format applicable to this codec
	 */
	virtual ERuntimeAudioFormat GetAudioFormat() const PURE_VIRTUAL(FBaseRuntimeCodec::GetAudioFormat, return ERuntimeAudioFormat::Invalid;)
};
