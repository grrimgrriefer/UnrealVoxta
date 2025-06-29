// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/**
 * ServerResponseChatUpdate
 * Read-only data struct containing the relevant data of the 'update' response from the VoxtaServer.
 * Contains message details including IDs and text content.
 */
struct ServerResponseChatUpdate : ServerResponseBase
{
#pragma region public API
public:
	/**
	 * Construct a chat update response.
	 *
	 * @param messageId The unique ID of the message.
	 * @param senderId The ID of the sender.
	 * @param text The text content of the message.
	 * @param sessionId The session ID this message belongs to.
	 */
	explicit ServerResponseChatUpdate(FGuid messageId,
			FGuid senderId,
			FStringView text,
			FGuid sessionId) : ServerResponseBase(ServerResponseType::ChatUpdate),
		MESSAGE_ID(messageId),
		SENDER_ID(senderId),
		TEXT_CONTENT(text),
		SESSION_ID(sessionId)
	{}
#pragma endregion

#pragma region data
public:
	/** The unique ID of the message. */
	const FGuid MESSAGE_ID;
	/** The ID of the sender. */
	const FGuid SENDER_ID;
	/** The text content of the message. */
	const FString TEXT_CONTENT;
	/** The session ID this message belongs to. */
	const FGuid SESSION_ID;
#pragma endregion
};
