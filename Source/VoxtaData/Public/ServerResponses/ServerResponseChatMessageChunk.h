// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseChatMessageBase.h"

/**
 * ServerResponseChatMessageChunk
 * Read-only data struct containing the relevant data of the 'replyChunk' response from the VoxtaServer.
 * Represents a chunk of a chat message, including sender, indices, text, audio path, and narration flag.
 */
struct ServerResponseChatMessageChunk : public ServerResponseChatMessageBase
{
#pragma region public API
public:
	/**
	 * Construct a chat message chunk response.
	 *
	 * @param messageId The unique ID of the message.
	 * @param senderId The ID of the sender.
	 * @param sessionId The session ID this message belongs to.
	 * @param startIndex The start index of this chunk in the message.
	 * @param endIndex The end index of this chunk in the message.
	 * @param messageText The text content of this chunk.
	 * @param audioUrlPath The audio URL path for this chunk.
	 * @param isNarration Whether this chunk is narration.
	 */
	explicit ServerResponseChatMessageChunk(FGuid messageId,
			FGuid senderId,
			FGuid sessionId,
			int startIndex,
			int endIndex,
			FStringView messageText,
			FStringView audioUrlPath,
			bool isNarration) :
		ServerResponseChatMessageBase(ChatMessageType::MessageChunk, messageId, sessionId),
		SENDER_ID(senderId),
		START_INDEX(startIndex),
		END_INDEX(endIndex),
		MESSAGE_TEXT(messageText),
		AUDIO_URL_PATH(audioUrlPath),
		IS_NARRATION(isNarration)
	{}
#pragma endregion

#pragma region data
public:
	/** The ID of the sender. */
	const FGuid SENDER_ID;
	/** The start index of this chunk in the message. */
	const int START_INDEX;
	/** The end index of this chunk in the message. */
	const int END_INDEX;
	/** The text content of this chunk. */
	const FString MESSAGE_TEXT;
	/** The audio URL path for this chunk. */
	const FString AUDIO_URL_PATH;
	/** Whether this chunk is narration. */
	const bool IS_NARRATION;
#pragma endregion
};