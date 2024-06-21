// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseChatMessageBase.h"

/// <summary>
/// Read-only data struct containing the relevant data of the 'replyCancelled' response
/// from the VoxtaServer.
/// </summary>
struct ServerResponseChatMessageCancelled : public IServerResponseChatMessageBase
{
public:
	explicit ServerResponseChatMessageCancelled(FStringView messageId, FStringView sessionId) :
		IServerResponseChatMessageBase(messageId, sessionId)
	{
	}

	/// <summary>
	/// Identifies the response type as MessageCancelled.
	/// </summary>
	/// <returns>Returns MessageType::MessageCancelled.</returns>
	MessageType GetMessageType() const final
	{
		return MessageType::MessageCancelled;
	}
};