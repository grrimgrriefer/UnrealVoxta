// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseType.h"

/**
 * ServerResponseBase
 * Abstract read-only data struct that all responses from the VoxtaServer derive from.
 * Used to ensure a streamlined and type-safe public API for server responses.
 * Contains the response type for identification.
 */
struct ServerResponseBase
{
#pragma region public API
public:
	virtual ~ServerResponseBase() = default;

	explicit ServerResponseBase(ServerResponseType responseType) :
		RESPONSE_TYPE(responseType)
	{}
#pragma endregion

#pragma region data
public:
	/** The type of this server response. */
	const ServerResponseType RESPONSE_TYPE;
};
