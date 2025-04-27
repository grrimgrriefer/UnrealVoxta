// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ChatMessage.h"
#include "VoxtaServiceType.h"
#include "VoxtaServiceData.h"
#include "AiCharData.h"
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
	 * @return An immutable reference to the chatMessage history.
	 */
	const TArray<FChatMessage>& GetChatMessages()
	{
		return m_chatMessages;
	}

	/**
	 * Used only as required data for some VoxtaServer API calls.
	 *
	 * @return The VoxtaServer assigned ID of this session.
	 */
	FGuid GetSessionId() const { return m_sessionId; }

	/**
	 * Used by VoxtaClient to know if it should notify audioplayback handlers, mic input, etc...
	 *
	 * @return The services that were enabled when the chatsession was started.
	 */
	const TMap<VoxtaServiceType, FVoxtaServiceData>& GetActiveServices() const
	{
		return m_services;
	}

	/**
	 * Update the context of the ongoing chat session.
	 *
	 * @param newContext The new context for the ongoing chat session.
	 */
	void UpdateContext(const FString& newContext)
	{
		m_chatContext = newContext;
	}

	/**
	 * Adds a new chat message to the session.
	 *
	 * @param message The chat message to add.
	 */
	void AddChatMessage(const FChatMessage& message)
	{
		m_chatMessages.Add(message);
	}

	/**
	 * Adds a new chat message to the session.
	 *
	 * @param message The chat message to add.
	 */
	void RemoveChatMessage(const FGuid& messageID)
	{
		int index = m_chatMessages.IndexOfByPredicate([messageID] (const FChatMessage& InItem)
		{
			return InItem.GetMessageId() == messageID;
		});

		if (index != INDEX_NONE)
		{
			m_chatMessages.RemoveAt(index);
		}
	}

	/**
	 * Fetches a raw pointer to the ChatMessage that maches the id given in the parameters.
	 *
	 * Note: The text & audio in this data is not guarenteed to be complete. Be aware that only after the
	 * id has been broadcasted by VoxtaClientCharMessageAddedEvent that the message is considered final.
	 *
	 * @param messageId The id of the chatmessage you want to retrieve.
	 *
	 * @return An immutable pointer to the chatmessage, or nullptr if it was not found.
	 */
	FChatMessage* GetChatMessageById(const FGuid& messageId)
	{
		return m_chatMessages.FindByPredicate([&messageId] (const FChatMessage& inItem)
		{
			return inItem.GetMessageId() == messageId;
		});
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
			FGuid chatId,
			FGuid sessionId,
			const TMap<VoxtaServiceType, FVoxtaServiceData>& services,
			FStringView chatContext) :
		m_chatId(chatId),
		m_sessionId(sessionId),
		m_chatContext(chatContext),
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
	FChatSession() {}
#pragma endregion

#pragma region data
private:
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Chat ID"))
	FGuid m_chatId;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Session ID"))
	FGuid m_sessionId;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Character IDs"))
	TArray<FGuid> m_characterIds;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Context"))
	FString m_chatContext;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Messages so far"))
	TArray<FChatMessage> m_chatMessages;

	TArray<const FAiCharData*> m_characters;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Enabled services"))
	TMap<VoxtaServiceType, FVoxtaServiceData> m_services;
#pragma endregion
};
