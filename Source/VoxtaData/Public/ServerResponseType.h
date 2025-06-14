// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * ServerResponseType
 * Contains the possible response types that can be reported by the VoxtaApiReponseHandler.
 *
 * Used to identify the response type of a reply that has been received by the VoxtaServer.
 * Can be fetched via that ServerResponseBase.GetType() of any instance deriving from it.
 */
UENUM()
enum class ServerResponseType : uint8
{
	Welcome					UMETA(DisplayName = "Welcome"),
	CharacterList			UMETA(DisplayName = "CharacterList"),
	ChatStarted				UMETA(DisplayName = "ChatStarted"),
	ChatMessage				UMETA(DisplayName = "ChatMessage"),
	ChatUpdate				UMETA(DisplayName = "ChatUpdate"),
	SpeechTranscription		UMETA(DisplayName = "SpeechTranscription"),
	Error					UMETA(DisplayName = "Error"),
	ContextUpdated			UMETA(DisplayName = "ContextUpdated"),
	ChatClosed				UMETA(DisplayName = "ChatClosed"),
	ChatSessionError		UMETA(DisplayName = "ChatSessionError"),
};