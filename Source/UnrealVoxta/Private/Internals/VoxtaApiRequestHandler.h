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

	FSignalRValue GetLoadScenariosListData() const;

	FSignalRValue GetLoadChatsListData(const FString& characterId, const FString& scenarioId = FString()) const;

	/**
	 * Retrieve a SignalR formatted message to request a new chat session to be started with a specific character.
	 *
	 * @param charData The full metadata of the character that you want to start a chat with.
	 * @param context Context that will be used for the chat (json value, I think?) TODO
	 *
	 * @return The SignalR formatted message containing the request.
	 */
	FSignalRValue GetStartChatRequestData(const FAiCharData* charData, const FString& context = FString()) const;

	FSignalRValue GetStopChatRequestData() const;

	/**
	 * Retrieve a SignalR formatted message to request the registration of a user-message to the chat.
	 *
	 * Note: By default, the addition of a user message will trigger an auto-reply from the AI character.
	 *
	 * @param sessionId The ChatSession::CHAT_ID of the currently active chat.
	 * @param userInputText The text that the server should consider as what the user said.
	 * @param generateReply Should VoxtaServer generate a reply from the AI character(s) after receiving this message.
	 * @param characterActionInference TODO
	 *
	 * @return The SignalR formatted message containing the request.
	 */
	FSignalRValue GetSendUserMessageData(const FString& sessionId, const FString& userInputText, bool generateReply,
		bool characterActionInference) const;

	FSignalRValue GetNotifyAudioPlaybackStartedData(const FString& sessionId, const FString& messageId, int startIndex,
	int endIndex, double duration) const;

	/**
	 * Retrieve a SignalR formatted message to inform the VoxtaServer that the playback is completed on the client.
	 * Meaning it can being listening to the audioSocket for speech.
	 *
	 * @param sessionId The ChatSession::CHAT_ID of the currently active chat.
	 * @param messageId The FChatMessage.GetMessageId of the message that is done with the playback.
	 *
	 * @return The SignalR formatted message containing the request.
	 */
	FSignalRValue GetNotifyAudioPlaybackCompletedData(const FString& sessionId, const FString& messageId) const;

	FSignalRValue GetUpdateContextRequestData(const TArray<FString>& actions, const FString& sessionId,
		const FString& context, const FString& contextKey) const;

	FSignalRValue GetRequestCharacterSpeechRequestData(const FString& sessionId, const FString& text) const;

	FSignalRValue SendRevertLastSentMessage(const FString& sessionId) const;

	FSignalRValue SendDeleteChat(const FString& chatId) const;

	FSignalRValue SetFlags(const FString& sessionId, const TArray<FString>& flags) const;

	/// TODO no clue what this does, need to figure it out before implementing it
	// SendDeployResource(MissingResource resource, string name, byte[] bytes)
	/*
	{
		var json = new JObject
		{
			["$type"] = "deployResource",
			["id"] = resource.Id,
			["data"] = new JObject
			{
				["$type"] = "base64",
				["name"] = name,
				["base64Data"] = Convert.ToBase64String(bytes),
			},
		};
		Send(json.ToString());
	}
	*/
#pragma endregion
};
