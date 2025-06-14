// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * VoxtaClientState
 * Contains the possible states that can be reported by the VoxtaClient when polled.
 *
 * Used by communication between the UVoxtaClient and external systems. (VoxtaAudioInput, blueprints, etc.)
 * Can be fetched via GetCurrentState or through the VoxtaClientStateChangedEvent.
 */
UENUM(BlueprintType, Category = "Voxta")
enum class VoxtaClientState : uint8
{
	Disconnected			UMETA(DisplayName = "Disconnected"),
	AttemptingToConnect		UMETA(DisplayName = "AttemptingToConnect"),
	Authenticated			UMETA(DisplayName = "Authenticated"),
	Idle					UMETA(DisplayName = "Idle"),
	StartingChat			UMETA(DisplayName = "StartingChat"),
	GeneratingReply			UMETA(DisplayName = "GeneratingReply"),
	AudioPlayback			UMETA(DisplayName = "AudioPlayback"),
	WaitingForUserResponse	UMETA(DisplayName = "WaitingForUserResponse"),
	Terminated				UMETA(DisplayName = "Terminated")
};