// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "SignalRValue.h"
#include "VoxtaData/Public/AiCharData.h"
#include "Misc/Guid.h"

class VoxtaApiRequestHandler
{
public:
	/// <summary>
	/// Retrieve a SignalR formatted message to authenticate the client with the server.
	/// </summary>
	/// <returns>The SignalR formatted message containing the request.</returns>
	FSignalRValue GetAuthenticateRequestData() const;

	/// <summary>
	/// Retrieve a SignalR formatted message to request the list of all characters available.
	/// </summary>
	/// <returns>The SignalR formatted message containing the request.</returns>
	FSignalRValue GetLoadCharactersListData() const;

	/// <summary>
	/// Retrieve a SignalR formatted message to request the loading of a specific character and mark it as 'active'.
	/// Note: Requesting the start of a chat is a seperate request, this only loads the character.
	/// </summary>
	/// <param name="characterId">The FCharDataBase::m_id of the character that you want to load.</param>
	/// <returns>The SignalR formatted message containing the request.</returns>
	FSignalRValue GetLoadCharacterRequestData(const FString& characterId) const;

	/// <summary>
	/// Retrieve a SignalR formatted message to request a new chat session to be started with a specific character.
	/// </summary>
	/// <param name="charData">The full metadata of the character that you want to start a chat with.</param>
	/// <returns>The SignalR formatted message containing the request.</returns>
	FSignalRValue GetStartChatRequestData(const FAiCharData* charData) const;

	/// <summary>
	/// Retrieve a SignalR formatted message to request the registration of a user-message to the chat.
	/// Note: By default, the addition of a user message will trigger an auto-reply from the AI character.
	/// </summary>
	/// <param name="sessionId">The ChatSession::m_chatId of the currently active chat.</param>
	/// <param name="userInputText">The text that the server should consider as what the user said.</param>
	/// <returns>The SignalR formatted message containing the request.</returns>
	FSignalRValue GetSendUserMessageData(const FString& sessionId, const FString& userInputText) const;

	/// <summary>
	/// Retrieve a SignalR formatted message to inform the VoxtaServer that the playback is completed on the client.
	/// Meaning it can being listening to the audioSocket for speech.
	/// </summary>
	/// <param name="sessionId">The ChatSession::m_chatId of the currently active chat.</param>
	/// <param name="messageId">The FChatMessage.GetMessageId of the message that is done with the playback.</param>
	/// <returns></returns>
	FSignalRValue GetNotifyAudioPlaybackCompleteData(const FString& sessionId, const FString& messageId) const;
};
