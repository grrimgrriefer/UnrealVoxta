// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/** Helper enum to keep track of what the AudioPlaybcakHandler is doing at the moment. */
UENUM()
enum class AudioPlaybackInternalState : uint8
{
	Idle,
	Playing,
	Done
};