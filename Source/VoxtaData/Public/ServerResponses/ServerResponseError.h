// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/**
 * Read-only data struct containing the relevant data of the 'error' response from the VoxtaServer.
 */
struct ServerResponseError : public ServerResponseBase
{
#pragma region public API
public:
	/** Create a deserialized version of the VoxtaServer response represents the 'Error' data. */
	explicit ServerResponseError(FStringView message,
			FStringView details) : ServerResponseBase(ServerResponseType::Error),
		ERROR_MESSAGE(message),
		ERROR_DETAILS(details)
	{
	}
#pragma endregion

#pragma region data
public:
	const FString ERROR_MESSAGE;
	const FString ERROR_DETAILS;
#pragma endregion
};