// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

struct ChatMessage
{
	FString m_text;
	TArray<FString> m_audioUrls;
	const FString m_messageId;
	const FString m_charId;

	explicit ChatMessage(FString messageId,
			FString charID,
			FString text) :
		m_text(text),
		m_messageId(messageId),
		m_charId(charID)
	{
	}

	explicit ChatMessage(FString messageId, FString charID) :
		m_messageId(messageId),
		m_charId(charID)
	{
	}
};