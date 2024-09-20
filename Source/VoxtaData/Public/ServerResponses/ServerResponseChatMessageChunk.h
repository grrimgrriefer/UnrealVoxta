// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseChatMessageBase.h"

/**
 * Read-only data struct containing the relevant data of the 'replyChunk' response from the VoxtaServer.
 */
struct ServerResponseChatMessageChunk : public ServerResponseChatMessageBase
{
#pragma region public API
public:
	/** Create a deserialized version of the VoxtaServer response represents the 'MessageChunk' data. */
	explicit ServerResponseChatMessageChunk(FStringView messageId,
			FStringView senderId,
			FStringView sessionId,
			int startIndex,
			int endIndex,
			FStringView messageText,
			FStringView audioUrlPath) :
		ServerResponseChatMessageBase(ChatMessageType::MessageChunk, messageId, sessionId),
		SENDER_ID(senderId),
		START_INDEX(startIndex),
		END_INDEX(endIndex),
		MESSAGE_TEXT(messageText),
		AUDIO_URL_PATH(audioUrlPath)
	{
	}
#pragma endregion

#pragma region data
public:
	const FString SENDER_ID;
	const int START_INDEX;
	const int END_INDEX;
	const FString MESSAGE_TEXT;
	const FString AUDIO_URL_PATH;
#pragma endregion
};