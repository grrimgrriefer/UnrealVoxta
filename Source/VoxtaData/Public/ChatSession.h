// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ChatMessage.h"
#include "VoxtaServiceData.h"

/// <summary>
/// Data struct containing all the relevant information regarding a chat session
/// between the user and AI characters.
/// </summary>
struct ChatSession
{
public:
	TArray<TUniquePtr<FChatMessage>> m_chatMessages;
	TArray<const FAiCharData*> m_characters;
	const FString m_chatId;
	const FString m_sessionId;
	// TODO: Add functionality for runtime disabling / enabling of services.
	const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData> m_services; //

	explicit ChatSession(const TArray<const FAiCharData*>& characters,
			FStringView chatId,
			FStringView sessionId,
			const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData>& services) :
		m_characters(characters),
		m_chatId(chatId),
		m_sessionId(sessionId),
		m_services(services)
	{
	}
};
