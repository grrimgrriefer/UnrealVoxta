// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseChatMessageBase.h"

/**
 * ServerResponseChatMessageStart
 * Read-only data struct containing the relevant data of the 'replyStart' response from the VoxtaServer.
 * Signals the start of a chat message with associated sender information.
 */
struct ServerResponseChatMessageStart : public ServerResponseChatMessageBase
{
#pragma region public API
public:
	/**
	 * Construct a chat message start response.
	 *
	 * @param messageId The unique ID of the message.
	 * @param senderId The ID of the sender.
	 * @param sessionId The session ID this message belongs to.
	 */
	explicit ServerResponseChatMessageStart(FGuid messageId,
			FGuid senderId,
			FGuid sessionId) :
		ServerResponseChatMessageBase(ChatMessageType::MessageStart, messageId, sessionId),
		SENDER_ID(senderId)
	{}
#pragma endregion

#pragma region data
public:
	/** The ID of the sender of the message with matching MESSAGE_ID. */
	const FGuid SENDER_ID;
#pragma endregion
};