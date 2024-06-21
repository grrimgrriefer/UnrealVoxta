// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "VoxtaData/Public/VoxtaServiceData.h"

/// <summary>
/// Read-only data struct containing the relevant data of the 'chatStarted' response
/// from the VoxtaServer.
/// </summary>
struct ServerResponseChatStarted : public IServerResponseBase
{
public:
	const TArray<FString> m_characterIds;
	const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData> m_services;
	const FString m_userId;
	const FString m_chatId;
	const FString m_sessionId;

	explicit ServerResponseChatStarted(FStringView userId,
			const TArray<FString>& characterIds,
			const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData>& services,
			FStringView chatId,
			FStringView sessionId) :
		m_characterIds(characterIds),
		m_services(services),
		m_userId(userId),
		m_chatId(chatId),
		m_sessionId(sessionId)
	{
	}

	/// <summary>
	/// Identifies the response type as ChatStarted.
	/// </summary>
	/// <returns>Returns MessageType::ChatStarted.</returns>
	ServerResponseType GetType() const final
	{
		return ServerResponseType::ChatStarted;
	}
};
