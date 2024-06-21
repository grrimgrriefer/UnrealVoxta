// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseChatMessageBase.h"

/// <summary>
/// Read-only data struct containing the relevant data of the 'replyStart' response
/// from the VoxtaServer.
/// </summary>
struct ServerResponseChatMessageStart : public IServerResponseChatMessageBase
{
public:
	const FString m_senderId;

	explicit ServerResponseChatMessageStart(FStringView messageId,
			FStringView senderId,
			FStringView sessionId) :
		IServerResponseChatMessageBase(messageId, sessionId),
		m_senderId(senderId)
	{
	}

	/// <summary>
	/// Identifies the response type as MessageStart.
	/// </summary>
	/// <returns>Returns MessageType::MessageStart.</returns>
	MessageType GetMessageType() const final
	{
		return MessageType::MessageStart;
	}
};