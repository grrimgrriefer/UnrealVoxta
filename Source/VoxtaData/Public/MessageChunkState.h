// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum class MessageChunkState : uint8
{
	Idle,
	Idle_Downloaded,
	Idle_Processed,
	Busy,
	ReadyForPlayback,
	CleanedUp
};