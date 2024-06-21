// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ChatMessage.generated.h"

/// <summary>
/// Data struct representing all the relevant information for a single chat message.
///
/// NOTE: The text and audioUrls are appended with new info as more chunks are received.
/// </summary>
USTRUCT()
struct FChatMessage
{
	GENERATED_BODY()

public:
	// TODO: Add flag to indicate the message is complete and no further chunks are expected.
	FString m_text;
	TArray<FString> m_audioUrls;

	FStringView GetMessageId() const { return m_messageId; };
	FStringView GetCharId() const { return m_charId; };

	explicit FChatMessage(FStringView messageId,
			FStringView charId,
			FStringView text) :
		m_text(text),
		m_messageId(messageId),
		m_charId(charId)
	{
	}

	explicit FChatMessage(FStringView messageId, FStringView charId) :
		m_messageId(messageId),
		m_charId(charId)
	{
	}

	explicit FChatMessage() {};

private:
	FString m_messageId;
	FString m_charId;
};