// Georgy Treshchev 2024.

#pragma once

#include "CoreMinimal.h"
#include "AudioStructs.h"
#include "ImportedSoundWave.h"

#include "RuntimeAudioImporterLibrary.generated.h"

UCLASS(BlueprintType, Category = "Imported Sound Wave")
class AUDIOUTILITY_API URuntimeAudioImporterLibrary : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAudioImportedEventCallback, FString, identifier, UImportedSoundWave*, soundWave);

	FAudioImportedEventCallback AudioImportedEvent;

	void ImportAudioFromBuffer(FString identifier, TArray64<uint8> buffer);
	void ImportAudioFromDecodedInfo(FString identifier, FDecodedAudioStruct&& DecodedAudioInfo);

	static bool ResampleAndMixChannelsInDecodedInfo(FDecodedAudioStruct& DecodedAudioInfo, uint32 NewSampleRate, uint32 NewNumOfChannels);
	static bool DecodeAudioData(FEncodedAudioStruct&& EncodedAudioInfo, FDecodedAudioStruct& DecodedAudioInfo);
	static bool EncodeAudioData(FDecodedAudioStruct&& DecodedAudioInfo, FEncodedAudioStruct& EncodedAudioInfo, uint8 Quality);

private:
	void OnResult_Internal(FString identifier, UImportedSoundWave* ImportedSoundWave);
};
