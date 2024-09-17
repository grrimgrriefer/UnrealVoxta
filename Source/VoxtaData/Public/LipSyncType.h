// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * LipSyncType
 * All the possible LipSync types that are currently supported by the UnrealVoxta client.
 *
 * Exposed by the VoxtaAudioPlayback to the UE interface. The user can define on a per-character basis what type of
 * lipsync is desired.
 *
 * Note: Changing lipsync-type during runtime is currently not supported.
 */
UENUM(BlueprintType, Category = "Voxta")
enum class LipSyncType : uint8
{
	None				UMETA(DisplayName = "None"),
	Custom				UMETA(DisplayName = "Custom"),
	OVRLipSync			UMETA(DisplayName = "OVRLipSync"),
	Audio2Face			UMETA(DisplayName = "Audio2Face")
};