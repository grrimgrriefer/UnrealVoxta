// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/// <summary>
/// The possible states reponses that can be returned by the VoxtaClient when polled.
/// </summary>
UENUM(BlueprintType)
enum class VoxtaClientState : uint8
{
	Disconnected			UMETA(DisplayName = "Disconnected"),
	AttemptingToConnect		UMETA(DisplayName = "AttemptingToConnect"),
	Authenticated			UMETA(DisplayName = "Authenticated"),
	Idle					UMETA(DisplayName = "Idle"),
	StartingChat			UMETA(DisplayName = "StartingChat"),
	GeneratingReply			UMETA(DisplayName = "GeneratingReply"),
	AudioPlayback			UMETA(DisplayName = "AudioPlayback"),
	WaitingForUserReponse	UMETA(DisplayName = "WaitingForUserReponse"),
	Terminated				UMETA(DisplayName = "Terminated")
};