// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "AudioStructs.h"
#include "ImportedSoundWave.h"

#include "AudioImporter.generated.h"

UCLASS(BlueprintType, Category = "Imported Sound Wave")
class AUDIOUTILITY_API UAudioImporter : public UObject
{
	GENERATED_BODY()

public:
	void ImportAudioFromBuffer(TArray64<uint8> buffer);
	bool DecodeAudioData(FEncodedAudioStruct&& EncodedAudioInfo, FDecodedAudioStruct& DecodedAudioInfo);
	void ImportAudioFromDecodedInfo(FDecodedAudioStruct&& DecodedAudioInfo);

	static bool ResampleAndMixChannelsInDecodedInfo(FDecodedAudioStruct& DecodedAudioInfo, uint32 NewSampleRate, uint32 NewNumOfChannels);
};
