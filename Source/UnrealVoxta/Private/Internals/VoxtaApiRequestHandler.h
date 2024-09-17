// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

class FSignalRValue;
struct FAiCharData;

/**
 * VoxtaApiRequestHandler
 * Internal helper class to serialize request data which then can be sent to VoxtaServer via SignalR.
 *
 * Note: All methods and fields must be const (immutable), as this class should remain stateless.
 */
class VoxtaApiRequestHandler
{
#pragma region public API
public:
	/**
	 * Retrieve a SignalR formatted message to authenticate the client with the server.
	 *
	 * @return The SignalR formatted message containing the request.
	 */
	FSignalRValue GetAuthenticateRequestData() const;

	/**
	 * Retrieve a SignalR formatted message to request the list of all characters available.
	 *
	 * @return The SignalR formatted message containing the request.
	 */
	FSignalRValue GetLoadCharactersListData() const;

	/**
	 * Retrieve a SignalR formatted message to request the loading of a specific character and mark it as 'active'.
	 *
	 * Note: Requesting the start of a chat is a seperate request, this only loads the character.
	 *
	 * @param characterId The FBaseCharData::m_id of the character that you want to load.
	 *
	 * @return The SignalR formatted message containing the request.
	 */
	FSignalRValue GetLoadCharacterRequestData(const FString& characterId) const;

	/**
	 * Retrieve a SignalR formatted message to request a new chat session to be started with a specific character.
	 *
	 * @param charData The full metadata of the character that you want to start a chat with.
	 *
	 * @return The SignalR formatted message containing the request.
	 */
	FSignalRValue GetStartChatRequestData(const FAiCharData* charData) const;

	/**
	 * Retrieve a SignalR formatted message to request the registration of a user-message to the chat.
	 *
	 * Note: By default, the addition of a user message will trigger an auto-reply from the AI character.
	 *
	 * @param sessionId The ChatSession::m_chatId of the currently active chat.
	 * @param userInputText The text that the server should consider as what the user said.
	 *
	 * @return The SignalR formatted message containing the request.
	 */
	FSignalRValue GetSendUserMessageData(const FString& sessionId, const FString& userInputText) const;

	/**
	 * Retrieve a SignalR formatted message to inform the VoxtaServer that the playback is completed on the client.
	 * Meaning it can being listening to the audioSocket for speech.
	 *
	 * @param sessionId The ChatSession::m_chatId of the currently active chat.
	 * @param messageId The FChatMessage.GetMessageId of the message that is done with the playback.
	 *
	 * @return The SignalR formatted message containing the request.
	 */
	FSignalRValue GetNotifyAudioPlaybackCompleteData(const FString& sessionId, const FString& messageId) const;
#pragma endregion
};
