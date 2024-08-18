// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#if WITH_OVRLIPSYNC
#include "LipSyncDataOVR.h"
#endif

class VOXTAUTILITY_API LipSyncGenerator
{
public:
#if WITH_OVRLIPSYNC
	static void GenerateOVRLipSyncData(const TArray<uint8>& rawAudioData, TFunction<void(ULipSyncDataOVR*)> callback);
#endif
};
