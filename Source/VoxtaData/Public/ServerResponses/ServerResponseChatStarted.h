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
public:

	explicit ServerResponseChatStarted(FStringView userId,
			const TArray<FString>& characterIds,
			const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData>& services,
			FStringView chatId,
			FStringView sessionId) : ServerResponseBase(ServerResponseType::ChatStarted),
		m_characterIds(characterIds),
		m_services(services),
		m_userId(userId),
		m_chatId(chatId),
		SESSION_ID(sessionId)
	{
	}

#pragma region data
public:
	const TArray<FString> m_characterIds;
	const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData> m_services;
	const FString m_userId;
	const FString m_chatId;
	const FString SESSION_ID;
#pragma endregion
};
