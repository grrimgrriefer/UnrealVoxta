// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "VoxtaData/Public/VoxtaServiceData.h"

/**
 * Read-only data struct containing the relevant data of the 'chatStarted' response from the VoxtaServer.
 */
struct ServerResponseChatStarted : public ServerResponseBase
{
#pragma region public API
public:
	/** Create a deserialized version of the VoxtaServer response represents the 'ChatStarted' data. */
	explicit ServerResponseChatStarted(FGuid userId,
			const TArray<FGuid>& characterIds,
			const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData>& services,
			FGuid chatId,
			FGuid sessionId,
			FStringView contextText) : ServerResponseBase(ServerResponseType::ChatStarted),
		CHARACTER_IDS(characterIds),
		SERVICES(services),
		USER_ID(userId),
		CHAT_ID(chatId),
		SESSION_ID(sessionId),
		CONTEXT_TEXT(contextText)
	{}
#pragma endregion

#pragma region data
public:
	const TArray<FGuid> CHARACTER_IDS;
	const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData> SERVICES;
	const FGuid USER_ID;
	const FGuid CHAT_ID;
	const FGuid SESSION_ID;
	const FString CONTEXT_TEXT;
#pragma endregion
};
