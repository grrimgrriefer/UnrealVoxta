// Georgy Treshchev 2024.

#pragma once

#include "CoreMinimal.h"
#include "AudioStructs.h"
#include "ImportedSoundWave.h"

class VOXTAAUDIOUTILITY_API URuntimeAudioImporterLibrary
{
public:
	static void ImportAudioFromBuffer(TArray64<uint8> buffer, TFunction<void(UImportedSoundWave*)> callback);
	static void ImportAudioFromDecodedInfo(FDecodedAudioStruct&& DecodedAudioInfo, TFunction<void(UImportedSoundWave*)> callback);

	static bool ResampleAndMixChannelsInDecodedInfo(FDecodedAudioStruct& DecodedAudioInfo, uint32 NewSampleRate, uint32 NewNumOfChannels);
	static bool DecodeAudioData(FEncodedAudioStruct&& EncodedAudioInfo, FDecodedAudioStruct& DecodedAudioInfo);
	static bool EncodeAudioData(FDecodedAudioStruct&& DecodedAudioInfo, FEncodedAudioStruct& EncodedAudioInfo, uint8 Quality);
};
