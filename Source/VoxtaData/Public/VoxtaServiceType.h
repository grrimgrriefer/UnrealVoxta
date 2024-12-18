// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * ServiceType
 * All the possible VoxtaServer Services that the UnrealVoxta client currently supports.
 * Used to fetch the correct version of the service from the FChatSession data container.
 */
UENUM(BlueprintType, Category = "Voxta")
enum class VoxtaServiceType : uint8
{
	TextGen,
	SpeechToText,
	TextToSpeech
};