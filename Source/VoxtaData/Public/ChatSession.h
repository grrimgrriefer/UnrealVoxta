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
 * Acts as the single source of truth for chat state, message history, and available services.
 * Thread-safe for concurrent read/write access.
 */
USTRUCT(BlueprintType, Category = "Voxta")
struct VOXTADATA_API FChatSession
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Get the chat message history for this session.
	 * Can be used to add, remove, and update chat message entries.
	 * Acts as the source-of-truth for what has been said so far.
	 *
	 * @return An immutable reference to the chat message history.
	 */
	const TArray<FChatMessage>& GetChatMessages()
	{
		return m_chatMessages;
	}

	/**
	 * Get the VoxtaServer assigned ID of this session.
	 * Used as required data for some VoxtaServer API calls.
	 *
	 * @return The session ID.
	 */
	FGuid GetSessionId() const { return m_sessionId; }

	/**
	 * Get the services that were enabled when the chat session was started.
	 * Used by VoxtaClient to know if it should notify audio playback handlers, mic input, etc.
	 *
	 * @return The map of active services.
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
	 * Add a new chat message to the session.
	 *
	 * @param message The chat message to add.
	 */
	void AddChatMessage(const FChatMessage& message)
	{
		m_chatMessages.Add(message);
	}

	/**
	 * Remove a chat message from the session by message ID.
	 *
	 * @param messageID The ID of the chat message to remove.
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
	 * Fetch a raw pointer to the ChatMessage that matches the given ID.
	 * Note: The text & audio in this data is not guaranteed to be complete until the message is finalized.
	 *
	 * @param messageId The ID of the chat message to retrieve.
	 * @return An immutable pointer to the chat message, or nullptr if not found.
	 */
	FChatMessage* GetChatMessageById(const FGuid& messageId)
	{
		return m_chatMessages.FindByPredicate([&messageId] (const FChatMessage& msg)
		{
			return msg.GetMessageId() == messageId;
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

	/** Default constructor */
	FChatSession() = default;

	/**
	 * Get the VoxtaServer assigned ID of this chat session.
	 * Used as the session identifier for HTTP requests and WebSocket messages.
	 *
	 * @return The ID of the chat.
	 */
	FGuid GetChatId() const { return m_chatId; }

	/**
	 * Get the current context of the chat session.
	 * Used to retrieve the current context text that influences the AI's responses.
	 *
	 * @return The current chat context string.
	 */
	FStringView GetChatContext() const { return m_chatContext; }

	/**
	 * Get the list of character IDs in the chat session.
	 * Used to identify which characters are participating.
	 *
	 * @return The array of character GUIDs.
	 */
	const TArray<FGuid>& GetCharacterIds() const { return m_characterIds; }
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
