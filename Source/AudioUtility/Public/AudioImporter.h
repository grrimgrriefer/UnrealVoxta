// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "AudioStructs.h"
#include "ImportedSoundWave.h"

class AUDIOUTILITY_API AudioImporter
{
public:
	void ImportAudioFromBuffer(TArray64<uint8> buffer);
	bool DecodeAudioData(FEncodedAudioStruct&& EncodedAudioInfo, FDecodedAudioStruct& DecodedAudioInfo);
	void ImportAudioFromDecodedInfo(FDecodedAudioStruct&& DecodedAudioInfo);
};
