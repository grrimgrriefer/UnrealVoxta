// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/**
 * ServerResponseError
 * Read-only data struct containing the relevant data of the 'error' response from the VoxtaServer.
 * Contains error message and detailed information about the error.
 */
struct ServerResponseError : public ServerResponseBase
{
#pragma region public API
public:
	/**
	 * Construct an error response.
	 *
	 * @param message The main error message.
	 * @param details The detailed error information.
	 */
	explicit ServerResponseError(FStringView message,
			FStringView details) : ServerResponseBase(ServerResponseType::Error),
		ERROR_MESSAGE(FString(message)),
		ERROR_DETAILS(FString(details))
	{}
#pragma endregion

#pragma region data
public:
	/** The main error message. */
	const FString ERROR_MESSAGE;
	/** The detailed error information. */
	const FString ERROR_DETAILS;
#pragma endregion
};