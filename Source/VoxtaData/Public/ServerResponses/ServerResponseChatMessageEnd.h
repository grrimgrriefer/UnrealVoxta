// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseChatMessageBase.h"

/**
 * Read-only data struct containing the relevant data of the 'replyEnd' response from the VoxtaServer.
 */
struct ServerResponseChatMessageEnd : public ServerResponseChatMessageBase
{
#pragma region public API
public:
	explicit ServerResponseChatMessageEnd(FGuid messageId,
			FGuid senderId,
			FGuid sessionId) :
		ServerResponseChatMessageBase(ChatMessageType::MessageEnd, messageId, sessionId),
		SENDER_ID(senderId)
	{}
#pragma endregion

#pragma region data
public:
	const FGuid SENDER_ID;
#pragma endregion
};