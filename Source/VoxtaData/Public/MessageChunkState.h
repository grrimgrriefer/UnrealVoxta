// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * MessageChunkState
 * All the possible states that an instance of MessageChunkAudioContainer could be in.
 *
 * Used for communication between an instance of MessageChunkAudioContainer and the VoxtaAudioPlayback that manages it.
 * Can be fetched via GetCurrentState of the MessageChunkAudioContainer instance
 */
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