// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ChatMessage.h"
#include "VoxtaServiceData.h"

struct ChatSession
{
public:
	TArray<TUniquePtr<FChatMessage>> m_chatMessages;
	TArray<const FAiCharData*> m_characters;
	const FString m_chatId;
	const FString m_sessionId;
	const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData> m_services; // Assumes we don't allow the user to disable / enable services while chatting

	explicit ChatSession(const TArray<const FAiCharData*>& characters,
			FString chatId,
			FString sessionId,
			const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData>& services) :
		m_characters(characters),
		m_chatId(chatId),
		m_sessionId(sessionId),
		m_services(services)
	{
	}
};
