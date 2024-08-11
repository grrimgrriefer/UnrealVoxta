// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncData.h"

class VOXTAUTILITY_API LipSyncGenerator
{
public:
	static void GenerateLipSync(const TArray<uint8>& rawAudioData, TFunction<void(FLipSyncData)> callback);

private:

#if WITH_OVRLIPSYNC
	static void GenerateOVRLipSyncData(const TArray<uint8>& rawAudioData, TFunction<void(FLipSyncData)> callback);
#endif
};
