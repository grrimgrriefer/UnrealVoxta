// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#if WITH_OVRLIPSYNC
#include "LipSyncDataOVR.h"
#endif
#include "LipSyncDataA2F.h"

class LipSyncGenerator
{
public:
#if WITH_OVRLIPSYNC
	static void GenerateOVRLipSyncData(const TArray<uint8>& rawAudioData, TFunction<void(ULipSyncDataOVR*)> callback);
#endif
	static void GenerateA2FLipSyncData(const TArray<uint8>& rawAudioData, Audio2FaceRESTHandler& A2FRestHandler, TFunction<void(ULipSyncDataA2F*)> callback);

private:
	static void WriteWavFile(const TArray<uint8>& rawAudioData, const FString& filePath);
};
