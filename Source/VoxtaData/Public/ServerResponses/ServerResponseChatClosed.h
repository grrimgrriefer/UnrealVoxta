// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/**
 * ServerResponseChatClosed
 * Read-only data struct containing the relevant data of the 'chatClosed' response from the VoxtaServer.
 * Contains chat and session identifiers for the closed chat.
 */
struct ServerResponseChatClosed : ServerResponseBase
{
#pragma region public API
public:
	/**
	 * Construct a chat closed response.
	 *
	 * @param chatId The ID of the closed chat.
	 * @param sessionId The session ID of the closed chat.
	 */
	explicit ServerResponseChatClosed(FGuid chatId,
			FGuid sessionId) : ServerResponseBase(ServerResponseType::ChatClosed),
		CHAT_ID(chatId),
		SESSION_ID(sessionId)
	{}
#pragma endregion

#pragma region data
public:
	/** The ID of the chat that is now closed. */
	const FGuid CHAT_ID;
	/** The session ID of the chat that is now closed. */
	const FGuid SESSION_ID;
#pragma endregion
};
