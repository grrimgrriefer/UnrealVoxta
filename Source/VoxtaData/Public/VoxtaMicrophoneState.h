// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/// <summary>
/// The possible states that the VoxtaAudioInput and report when polled.
/// </summary>
UENUM(BlueprintType)
enum class VoxtaMicrophoneState : uint8
{
	NotConnected			UMETA(DisplayName = "NotConnected"),
	Initializing			UMETA(DisplayName = "Initializing"),
	Ready					UMETA(DisplayName = "Ready"),
	InUse					UMETA(DisplayName = "InUse"),
	Closed					UMETA(DisplayName = "Closed")
};