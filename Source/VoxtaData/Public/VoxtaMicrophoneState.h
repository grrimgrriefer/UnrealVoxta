// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * MicrophoneState
 * To keep track of what's going on with the VoxtaAudioInput internals, as well as user-friendly logging.
 */
UENUM(BlueprintType, Category = "Voxta")
enum class VoxtaMicrophoneState : uint8
{
	Uninitialized,
	NotConnected,
	Connecting,
	Ready,
	InUse
};