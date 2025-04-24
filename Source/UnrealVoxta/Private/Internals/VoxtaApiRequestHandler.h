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
	static FSignalRValue GetAuthenticateRequestData();

	/**
	 * Retrieve a SignalR formatted message to request the list of all characters available.
	 *
	 * @return The SignalR formatted message containing the request.
	 */
	static FSignalRValue GetLoadCharactersListData();

	// TODO
	static FSignalRValue GetLoadScenariosListData();

	// TODO
	static FSignalRValue GetLoadChatsListData(const FGuid& characterId, const FGuid& scenarioId = FGuid());

	/**
	 * Retrieve a SignalR formatted message to request a new chat session to be started with a specific character.
	 *
	 * @param charData The full metadata of the character that you want to start a chat with.
	 * @param context Context that will be used for the chat (json value, I think?) TODO
	 *
	 * @return The SignalR formatted message containing the request.
	 */
	static FSignalRValue GetStartChatRequestData(const FAiCharData* charData, const FString& context = FString());

	// TODO
	static FSignalRValue GetStopChatRequestData();

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
	static FSignalRValue GetSendUserMessageData(const FGuid& sessionId, const FString& userInputText, bool generateReply,
		bool characterActionInference);

	// TODO
	static FSignalRValue GetNotifyAudioPlaybackStartedData(const FGuid& sessionId, const FGuid& messageId, int startIndex,
		int endIndex, double duration);

	/**
	 * Retrieve a SignalR formatted message to inform the VoxtaServer that the playback is completed on the client.
	 * Meaning it can being listening to the audioSocket for speech.
	 *
	 * @param sessionId The ChatSession::CHAT_ID of the currently active chat.
	 * @param messageId The FChatMessage.GetMessageId of the message that is done with the playback.
	 *
	 * @return The SignalR formatted message containing the request.
	 */
	static FSignalRValue GetNotifyAudioPlaybackCompletedData(const FGuid& sessionId, const FGuid& messageId);

	// TODO
	static FSignalRValue GetUpdateContextRequestData(/*const TArray<FString>& actions,*/ const FGuid& sessionId,
		const FString& context);

	// TODO
	static FSignalRValue GetRequestCharacterSpeechRequestData(const FGuid& sessionId, const FString& text);

	// TODO
	static FSignalRValue SendRevertLastSentMessage(const FGuid& sessionId);

	// TODO
	static FSignalRValue SendDeleteChat(const FGuid& chatId);

	// TODO
	static FSignalRValue SetFlags(const FGuid& sessionId, const TArray<FString>& flags);
#pragma endregion
};
