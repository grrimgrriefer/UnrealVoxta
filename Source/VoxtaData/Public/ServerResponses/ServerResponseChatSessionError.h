// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/**
 * ServerResponseChatSessionError
 * Read-only data struct containing the relevant data of the 'chatSessionError' response from the VoxtaServer.
 * Contains error details including session ID, retry flag, and error message.
 */
struct ServerResponseChatSessionError : public ServerResponseBase
{
#pragma region public API
public:
	/**
	 * Construct a chat session error response.
	 *
	 * @param chatSessionId The ID of the chat session that encountered the error.
	 * @param retry Whether the operation can be retried.
	 * @param message The error message.
	 */
	explicit ServerResponseChatSessionError(FStringView chatSessionId,
			bool retry, FStringView message) : ServerResponseBase(ServerResponseType::ChatSessionError),
		ERROR_CHAT_SESSION_ID(FString(chatSessionId)),
		ERROR_RETRY(retry),
		ERROR_MESSAGE(FString(message))
	{}
#pragma endregion

#pragma region data
public:
	/** The ID of the chat session that encountered the error. */
	const FString ERROR_CHAT_SESSION_ID;
	/** Whether the operation can be retried. */
	const bool ERROR_RETRY;
	/** The error message. */
	const FString ERROR_MESSAGE;
#pragma endregion
};