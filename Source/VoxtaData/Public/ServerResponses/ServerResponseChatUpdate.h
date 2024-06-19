// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

struct ServerResponseChatUpdate : IServerResponseBase
{
public:
	const FString m_messageId;
	const FString m_senderId;
	const FString m_text;
	const FString m_sessionId;

	explicit ServerResponseChatUpdate(FString messageId,
			FString senderId,
			FString text,
			FString sessionId) :
		m_messageId(messageId),
		m_senderId(senderId),
		m_text(text),
		m_sessionId(sessionId)
	{
	}

	ServerResponseType GetType() final
	{
		return ServerResponseType::ChatUpdate;
	}
};
