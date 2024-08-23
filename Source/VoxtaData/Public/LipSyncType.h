// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class LipSyncType : uint8
{
	None				UMETA(DisplayName = "None"),
	Custom				UMETA(DisplayName = "Custom"),
	OVRLipSync			UMETA(DisplayName = "OVRLipSync"),
	Audio2Face			UMETA(DisplayName = "Audio2Face")
};