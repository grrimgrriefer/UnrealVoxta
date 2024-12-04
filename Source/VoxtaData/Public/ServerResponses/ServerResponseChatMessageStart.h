// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseChatMessageBase.h"

/**
 * Read-only data struct containing the relevant data of the 'replyStart' response from the VoxtaServer.
 */
struct ServerResponseChatMessageStart : public ServerResponseChatMessageBase
{
#pragma region public API
public:
	/** Create a deserialized version of the VoxtaServer response represents the 'MessageStart' data. */
	explicit ServerResponseChatMessageStart(FGuid messageId,
			FGuid senderId,
			FGuid sessionId) :
		ServerResponseChatMessageBase(ChatMessageType::MessageStart, messageId, sessionId),
		SENDER_ID(senderId)
	{}
#pragma endregion

#pragma region data
public:
	const FGuid SENDER_ID;
#pragma endregion
};