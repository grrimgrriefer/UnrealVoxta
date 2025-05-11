// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "VoxtaData/Public/VoxtaServiceData.h"

/**
 * ServerResponseChatStarted
 * Read-only data struct containing the relevant data of the 'chatStarted' response from the VoxtaServer.
 * Contains user ID, character IDs, services, chat/session IDs, and context text.
 */
struct ServerResponseChatStarted : public ServerResponseBase
{
#pragma region public API
public:
	/**
	 * Construct a chat started response.
	 *
	 * @param userId The user ID.
	 * @param characterIds The list of character IDs in the chat.
	 * @param services The map of enabled services.
	 * @param chatId The chat ID.
	 * @param sessionId The session ID.
	 * @param contextText The context text for the chat.
	 */
	explicit ServerResponseChatStarted(FGuid userId,
		const TArray<FGuid>& characterIds,
		const TMap<VoxtaServiceType, FVoxtaServiceData>& services,
		FGuid chatId,
		FGuid sessionId,
		FStringView contextText) : ServerResponseBase(ServerResponseType::ChatStarted),
		CHARACTER_IDS(characterIds),
		SERVICES(services),
		USER_ID(userId),
		CHAT_ID(chatId),
		SESSION_ID(sessionId),
		CONTEXT_TEXT(contextText)
	{}
#pragma endregion

#pragma region data
public:
	/** The list of character IDs in the chat. */
	const TArray<FGuid> CHARACTER_IDS;
	/** The map of enabled services. */
	const TMap<VoxtaServiceType, FVoxtaServiceData> SERVICES;
	/** The user ID. */
	const FGuid USER_ID;
	/** The chat ID. */
	const FGuid CHAT_ID;
	/** The session ID. */
	const FGuid SESSION_ID;
	/** The context text for the chat. */
	const FString CONTEXT_TEXT;
#pragma endregion
};
