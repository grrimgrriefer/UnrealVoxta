// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseType.h"

/**
 * Abstract read-only data struct that all responses from the VoxtaServer derive from.
 * Main use for this is to ensure a streamlined public API.
 */
struct ServerResponseBase
{
#pragma region public API
public:
	virtual ~ServerResponseBase() = default;

	explicit ServerResponseBase(ServerResponseType responseType) :
		RESPONSE_TYPE(responseType)
	{
	}
#pragma endregion

#pragma region data
public:
	const ServerResponseType RESPONSE_TYPE;
#pragma endregion
};
