// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
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

class VOXTADATA_API ServerResponseBase
{
public:
	virtual ~ServerResponseBase() = default;
	virtual ServerResponseType GetType() = 0;
};
