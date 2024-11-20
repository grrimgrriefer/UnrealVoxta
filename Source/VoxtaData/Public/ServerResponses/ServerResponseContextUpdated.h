// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "VoxtaData/Public/VoxtaServiceData.h"

/**
 * Read-only data struct containing the relevant data of the 'chatStarted' response from the VoxtaServer.
 */
struct ServerResponseContextUpdated : public ServerResponseBase
{
#pragma region public API
public:
	/** Create a deserialized version of the VoxtaServer response represents the 'ChatStarted' data. */
	explicit ServerResponseContextUpdated(FStringView userId,
			const TArray<FString>& characterIds,
			const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData>& services,
			FStringView chatId,
			FStringView sessionId) : ServerResponseBase(ServerResponseType::ContextUpdated),
		CHARACTER_IDS(characterIds),
		SERVICES(services),
		USER_ID(userId),
		CHAT_ID(chatId),
		SESSION_ID(sessionId)
	{}
#pragma endregion

#pragma region data
public:
	const TArray<FString> CHARACTER_IDS;
	const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData> SERVICES;
	const FString USER_ID;
	const FString CHAT_ID;
	const FString SESSION_ID;
#pragma endregion
};
