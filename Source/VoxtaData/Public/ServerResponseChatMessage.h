// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

struct ServerResponseChatMessage : ServerResponseBase
{
	enum class MessageType
	{
		MESSAGE_START,
		MESSAGE_CHUNK,
		MESSAGE_END
	};

	const MessageType m_messageType;
	const FString m_messageId;
	const FString m_senderId;
	const FString m_sessionId;
	const int m_startIndex = 0;
	const int m_endIndex = 0;
	const FString m_messageText = "";
	const FString m_audioUrlPath = "";

	explicit ServerResponseChatMessage(MessageType type,
			FString messageId,
			FString senderId,
			FString sessionId) :
		m_messageType(type),
		m_messageId(messageId),
		m_senderId(senderId),
		m_sessionId(sessionId)
	{
	}

	explicit ServerResponseChatMessage(MessageType type,
			FString messageId,
			FString senderId,
			FString sessionId,
			int startIndex,
			int endIndex,
			FString messageText,
			FString audioUrlPath) :
		m_messageType(type),
		m_messageId(messageId),
		m_senderId(senderId),
		m_sessionId(sessionId),
		m_startIndex(startIndex),
		m_endIndex(endIndex),
		m_messageText(messageText),
		m_audioUrlPath(audioUrlPath)
	{
	}

	ServerResponseType GetType() final
	{
		return ServerResponseType::ChatMessage;
	}
};