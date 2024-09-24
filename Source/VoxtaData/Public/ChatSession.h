// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ChatMessage.h"
#include "VoxtaServiceData.h"
#include "ChatSession.generated.h"

/**
 * FChatSession
 * Data struct containing all the relevant information regarding a chat session between the user and AI characters.
 *
 * Note: Contains both immutable and stateful data & acts as the single source-of-truth regarding the ongoing chat.
 * Can be fetched via GetChatSession() of the VoxtaClient.
 */
USTRUCT(BlueprintType, Category = "Voxta")
struct VOXTADATA_API FChatSession
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Can be used to add, remove and update chatMessage entries.
	 * Acts as the source-of-truth of whatever has been said so far.
	 *
	 * @return A direct reference to the chatMessage history.
	 */
	TArray<FChatMessage>& GetChatMessages()
	{
		return m_chatMessages;
	}

	/**
	 * Used only as required data for some VoxtaServer API calls.
	 *
	 * @return The VoxtaServer assigned ID of this session.
	 */
	FString GetSessionId() const { return m_sessionId; }

	/**
	 * Used by VoxtaClient to know if it should notify audioplayback handlers, mic input, etc...
	 *
	 * @return The currently enabled services on VoxtaServer (not really, just what was active when starting chat) TODO
	 */
	const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData>& GetActiveServices() const
	{
		return m_services;
	}

	/**
	 * Create a new instance of the ChatSession, containing all relevant data to it.
	 *
	 * @param characters The AIcharacters participating in the chat session.
	 * @param chatId The VoxtaServer assigned id of this Chat.
	 * @param sessionId The VoxtaServer assigned id of this Session.
	 * @param services The VoxtaServer services that are enabled for this chat session.
	 */
	explicit FChatSession(const TArray<const FAiCharData*>& characters,
			FStringView chatId,
			FStringView sessionId,
			const TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData>& services) :
		m_chatId(chatId),
		m_sessionId(sessionId),
		m_characters(characters),
		m_services(services)
	{
		m_characterIds.Reserve(characters.Num());
		for (const FAiCharData* character : characters)
		{
			m_characterIds.Emplace(character->GetId());
		}
	}

	/** Default constructor, should not be used manually, but is enforced by Unreal */
	explicit FChatSession() {};
#pragma endregion

#pragma region data
private:
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Chat ID"))
	FString m_chatId;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Session ID"))
	FString m_sessionId;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Character IDs"))
	TArray<FString> m_characterIds;

	TArray<FChatMessage> m_chatMessages;
	TArray<const FAiCharData*> m_characters;
	// TODO: Add functionality for runtime disabling / enabling of services.
	TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData> m_services;
#pragma endregion
};
