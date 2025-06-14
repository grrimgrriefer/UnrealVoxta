// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/**
 * Read-only data struct containing the relevant data of the 'ContextUpdated' response from the VoxtaServer.
 */
struct ServerResponseContextUpdated : public ServerResponseBase
{
#pragma region public API
public:
	/** Create a deserialized version of the VoxtaServer response represents the 'contextUpdated' data. */
	explicit ServerResponseContextUpdated(FStringView contextText,
			FGuid sessionId) : ServerResponseBase(ServerResponseType::ContextUpdated),
		CONTEXT_TEXT(contextText),
		SESSION_ID(sessionId)
	{}
#pragma endregion

#pragma region data
public:
	const FString CONTEXT_TEXT;
	const FGuid SESSION_ID;
#pragma endregion
};
