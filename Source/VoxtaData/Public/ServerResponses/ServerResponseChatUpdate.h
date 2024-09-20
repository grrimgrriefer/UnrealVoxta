// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/**
 * Read-only data struct containing the relevant data of the 'update' response from the VoxtaServer.
 */
struct ServerResponseChatUpdate : ServerResponseBase
{
#pragma region public API
public:
	/** Create a deserialized version of the VoxtaServer response represents the 'ChatUpdate' data. */
	explicit ServerResponseChatUpdate(FStringView messageId,
			FStringView senderId,
			FStringView text,
			FStringView sessionId) : ServerResponseBase(ServerResponseType::ChatUpdate),
		MESSAGE_ID(messageId),
		SENDER_ID(senderId),
		TEXT_CONTENT(text),
		SESSION_ID(sessionId)
	{
	}
#pragma endregion

#pragma region data
public:
	const FString MESSAGE_ID;
	const FString SENDER_ID;
	const FString TEXT_CONTENT;
	const FString SESSION_ID;
#pragma endregion
};
