// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/// <summary>
/// Read-only data struct containing the relevant data of the 'update' response
/// from the VoxtaServer.
/// </summary>
struct ServerResponseChatUpdate : IServerResponseBase
{
public:
	const FString m_messageId;
	const FString m_senderId;
	const FString m_text;
	const FString m_sessionId;

	explicit ServerResponseChatUpdate(FStringView messageId,
			FStringView senderId,
			FStringView text,
			FStringView sessionId) :
		m_messageId(messageId),
		m_senderId(senderId),
		m_text(text),
		m_sessionId(sessionId)
	{
	}

	/// <summary>
	/// Identifies the response type as ChatUpdate.
	/// </summary>
	/// <returns>Returns MessageType::ChatUpdate.</returns>
	ServerResponseType GetType() const final
	{
		return ServerResponseType::ChatUpdate;
	}
};
