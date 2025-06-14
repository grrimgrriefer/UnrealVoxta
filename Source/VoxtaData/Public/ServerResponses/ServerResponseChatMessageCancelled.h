// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseChatMessageBase.h"

/**
 * Read-only data struct containing the relevant data of the 'replyCancelled' response from the VoxtaServer.
 */
struct ServerResponseChatMessageCancelled : public ServerResponseChatMessageBase
{
#pragma region public API
public:
	/** Create a deserialized version of the VoxtaServer response represents the 'MessageCancelled' data. */
	explicit ServerResponseChatMessageCancelled(FGuid messageId, FGuid sessionId) :
		ServerResponseChatMessageBase(ChatMessageType::MessageCancelled, messageId, sessionId)
	{}
#pragma endregion
};