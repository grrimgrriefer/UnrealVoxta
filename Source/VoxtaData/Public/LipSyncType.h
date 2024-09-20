// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * LipSyncType
 * All the possible LipSync types that are currently supported by the UnrealVoxta client.
 *
 * Used to expose the setting to the UE interface inspector, so the developer can define on a per-character basis what
 * type of lipsync is desired.
 *
 * Is set permanently on creation of the VoxtaAudioPlayback and cannot be fetched via the public API. However, you can
 * request it from the MessageChunkAudioContainer via the public const LIP_SYNC_TYPE.
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