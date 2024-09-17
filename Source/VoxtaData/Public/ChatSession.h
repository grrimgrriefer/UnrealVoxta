// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ChatMessage.h"
#include "VoxtaServiceData.h"
#include "ChatSession.generated.h"

/// <summary>
/// Data struct containing all the relevant information regarding a chat session
/// between the user and AI characters.
/// </summary>
USTRUCT(BlueprintType, Category = "Voxta")
struct VOXTADATA_API FChatSession
{
	GENERATED_BODY()

public:
	TArray<FChatMessage> m_chatMessages;

	FString GetSessionId() const { return m_sessionId; }

	explicit FChatSession(const TArray<const FAiCharData*>& characters,
			FStringView chatId,
			FStringView sessionId,
			const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData>& services) :
		m_chatId(chatId),
		m_sessionId(sessionId),
		m_services(services),
		m_characters(characters)
	{
		m_characterIds.Reserve(characters.Num());
		for (const FAiCharData* character : characters)
		{
			m_characterIds.Emplace(character->GetId());
		}
	}

	explicit FChatSession() {};

#pragma region data
private:
	UPROPERTY(BlueprintReadOnly, Category = "Voxta",
		meta = (AllowPrivateAccess = "true", DisplayName = "Chat ID"))
	FString m_chatId;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta",
		meta = (AllowPrivateAccess = "true", DisplayName = "Session ID"))
	FString m_sessionId;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta",
		meta = (AllowPrivateAccess = "true", DisplayName = "Character IDs"))
	TArray<FString> m_characterIds;

	// TODO: Add functionality for runtime disabling / enabling of services.
	TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData> m_services;
	TArray<const FAiCharData*> m_characters;
#pragma endregion
};
