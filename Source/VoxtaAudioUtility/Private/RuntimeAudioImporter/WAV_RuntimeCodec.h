// Georgy Treshchev 2024.

#pragma once

#include "CoreMinimal.h"
#include "BaseRuntimeCodec.h"

/**
 * WAV format codec implementation for runtime audio importing.
 * Handles decoding of WAV audio files and fixing potential format issues.
 */
class FWAV_RuntimeCodec : public FBaseRuntimeCodec
{
public:
	//~ Begin FBaseRuntimeCodec Interface
	virtual bool CheckAndFixAudioFormat(FRuntimeBulkDataBuffer<uint8>& AudioData) override;
	virtual bool Decode(FEncodedAudioStruct EncodedData, FDecodedAudioStruct& DecodedData) override;
	virtual ERuntimeAudioFormat GetAudioFormat() const override { return ERuntimeAudioFormat::Wav; }
	//~ End FBaseRuntimeCodec Interface
};
