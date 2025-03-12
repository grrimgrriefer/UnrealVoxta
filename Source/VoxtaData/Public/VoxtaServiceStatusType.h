// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType, Category = "Voxta")
enum class VoxtaServiceStatusType : uint8
{
	Unavailable,
	Inactive,
	Active
};