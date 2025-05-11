// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * VoxtaServiceStatusType
 * Represents the possible states of a Voxta service (like TextGen, TTS, STT).
 * Used to track whether services are available and active in the current session.
 */
UENUM(BlueprintType, Category = "Voxta")
enum class VoxtaServiceStatusType : uint8
{
	Unavailable,
	Inactive,
	Active
};