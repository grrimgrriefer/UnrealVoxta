// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * ServerResponseType
 * Used to identify the response type of a reply that has been received by the VoxtaServer.
 *
 * Technically not necessary, but makes maintenance a lot easier.
 * Can be fetched via that IServerResponseBase.GetType() of any instance deriving from it.
 */
UENUM()
enum class ServerResponseType : uint8
{
	Welcome					UMETA(DisplayName = "Welcome"),
	CharacterList			UMETA(DisplayName = "CharacterList"),
	CharacterLoaded			UMETA(DisplayName = "CharacterLoaded"),
	ChatStarted				UMETA(DisplayName = "ChatStarted"),
	ChatMessage				UMETA(DisplayName = "ChatMessage"),
	ChatUpdate				UMETA(DisplayName = "ChatUpdate"),
	SpeechTranscription		UMETA(DisplayName = "SpeechTranscription")
};