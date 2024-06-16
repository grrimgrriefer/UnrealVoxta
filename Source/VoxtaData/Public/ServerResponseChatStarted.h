// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "VoxtaData/Public/VoxtaServiceData.h"

struct ServerResponseChatStarted : public ServerResponseBase
{
	const TArray<FString> m_characterIds;
	const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData> m_services;
	const FString m_userId;
	const FString m_chatId;
	const FString m_sessionId;

	explicit ServerResponseChatStarted(FString userId,
			const TArray<FString>& characterIds,
			const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData>& services,
			FString chatId,
			FString sessionId) :
		m_characterIds(characterIds),
		m_services(services),
		m_userId(userId),
		m_chatId(chatId),
		m_sessionId(sessionId)
	{
	}

	ServerResponseType GetType() final
	{
		return ServerResponseType::ChatStarted;
	}
};
