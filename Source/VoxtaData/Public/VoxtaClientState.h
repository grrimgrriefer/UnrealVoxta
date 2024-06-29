// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/// <summary>
/// The possible states reponses that can be returned by the VoxtaClient when polled.
/// </summary>
UENUM(BlueprintType)
enum class VoxtaClientState : uint8
{
	Disconnected		UMETA(DisplayName = "Disconnected"),
	Connecting			UMETA(DisplayName = "Connecting"),
	Authenticated		UMETA(DisplayName = "Authenticated"),
	CharacterLobby		UMETA(DisplayName = "CharacterLobby"),
	StartingChat		UMETA(DisplayName = "StartingChat"),
	Chatting			UMETA(DisplayName = "Chatting"),
	Terminated			UMETA(DisplayName = "Terminated")
};