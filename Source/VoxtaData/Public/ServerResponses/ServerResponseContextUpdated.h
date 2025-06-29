// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/**
 * ServerResponseContextUpdated
 * Read-only data struct containing the relevant data of the 'ContextUpdated' response from the VoxtaServer.
 * Contains updated context text and session ID.
 */
struct ServerResponseContextUpdated : public ServerResponseBase
{
#pragma region public API
public:
	/**
	 * Construct a context updated response.
	 *
	 * @param contextText The updated context text.
	 * @param sessionId The session ID for which the context was updated.
	 */
	explicit ServerResponseContextUpdated(FStringView contextText,
			FGuid sessionId) : ServerResponseBase(ServerResponseType::ContextUpdated),
		CONTEXT_TEXT(contextText),
		SESSION_ID(sessionId)
	{}
#pragma endregion

#pragma region data
public:
	/** The updated context text. */
	const FString CONTEXT_TEXT;
	/** The session ID for which the context was updated. */
	const FGuid SESSION_ID;
#pragma endregion
};
