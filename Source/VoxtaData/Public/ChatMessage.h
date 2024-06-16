// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ChatMessage.generated.h"

USTRUCT()
struct FChatMessage
{
	GENERATED_BODY()

	FString m_text = "";
	TArray<FString> m_audioUrls = {};
	FString m_messageId = "";
	FString m_charId = "";

	FChatMessage()
	{
	};

	explicit FChatMessage(FString messageId,
			FString charID,
			FString text) :
		m_text(text),
		m_messageId(messageId),
		m_charId(charID)
	{
	}

	explicit FChatMessage(FString messageId, FString charID) :
		m_messageId(messageId),
		m_charId(charID)
	{
	}
};