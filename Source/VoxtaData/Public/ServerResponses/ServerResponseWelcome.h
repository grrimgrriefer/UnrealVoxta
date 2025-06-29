// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "UserCharData.h"

/**
 * ServerResponseWelcome
 * Read-only data struct containing the relevant data of the 'welcome' response from the VoxtaServer.
 * Contains user data, assistant ID, and version information.
 */
struct ServerResponseWelcome : public ServerResponseBase
{
#pragma region public API
public:
	/**
	 * Construct a welcome response.
	 *
	 * @param userData The user and character data.
	 * @param assistantId The ID of the assistant.
	 * @param voxtaServerVersion The version of the Voxta server.
	 * @param apiVersion The version of the API.
	 */
	explicit ServerResponseWelcome(const FUserCharData& userData, const FGuid assistantId,
		const FString& voxtaServerVersion, const FString& apiVersion) :
		ServerResponseBase(ServerResponseType::Welcome),
		USER_DATA(userData),
		ASSISTANT_ID(assistantId),
		SERVER_VERSION(voxtaServerVersion),
		API_VERSION(apiVersion)
	{}
#pragma endregion

#pragma region data
public:
	/** The user and character data. */
	const FUserCharData USER_DATA;
	/** The ID of the assistant. */
	const FGuid ASSISTANT_ID;
	/** The version of the Voxta server. */
	const FString SERVER_VERSION;
	/** The version of the server API. */
	const FString API_VERSION;
#pragma endregion
};
