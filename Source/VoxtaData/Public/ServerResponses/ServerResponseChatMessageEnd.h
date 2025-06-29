// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseChatMessageBase.h"

/**
 * ServerResponseChatMessageEnd
 * Read-only data struct containing the relevant data of the 'replyEnd' response from the VoxtaServer.
 * Signals the end of a chat message with associated sender information.
 */
struct ServerResponseChatMessageEnd : public ServerResponseChatMessageBase
{
#pragma region public API
public:
	/**
	 * Construct a chat message end response.
	 *
	 * @param messageId The unique ID of the message.
	 * @param senderId The ID of the sender.
	 * @param sessionId The session ID this message belongs to.
	 */
	explicit ServerResponseChatMessageEnd(FGuid messageId,
			FGuid senderId,
			FGuid sessionId) :
		ServerResponseChatMessageBase(ChatMessageType::MessageEnd, messageId, sessionId),
		SENDER_ID(senderId)
	{}
#pragma endregion

#pragma region data
public:
	/** The ID of the sender of the message with matching MESSAGE_ID. */
	const FGuid SENDER_ID;
#pragma endregion
};