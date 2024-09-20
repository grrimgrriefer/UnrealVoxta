// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "UserCharData.h"

/**
 * Read-only data struct containing the relevant data of the 'welcome' response from the VoxtaServer.
 */
struct ServerResponseWelcome : public ServerResponseBase
{
#pragma region public API
public:
	/** Create a deserialized version of the VoxtaServer response represents the 'Welcome' data. */
	explicit ServerResponseWelcome(const FUserCharData& userData) : ServerResponseBase(ServerResponseType::Welcome),
		USER_DATA(userData)
	{
	}
#pragma endregion

#pragma region data
public:
	const FUserCharData USER_DATA;
#pragma endregion
};
