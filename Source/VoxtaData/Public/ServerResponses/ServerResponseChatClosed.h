// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/**
 * Read-only data struct containing the relevant data of the 'chatClosed' response from the VoxtaServer.
 */
struct ServerResponseChatClosed : ServerResponseBase
{
#pragma region public API
public:
	/** Create a deserialized version of the VoxtaServer response represents the 'ChatClosed' data. */
	explicit ServerResponseChatClosed(FGuid chatId,
			FGuid sessionId) : ServerResponseBase(ServerResponseType::ChatClosed),
		CHAT_ID(chatId),
		SESSION_ID(sessionId)
	{}
#pragma endregion

#pragma region data
public:
	const FGuid CHAT_ID;
	const FGuid SESSION_ID;
#pragma endregion
};
