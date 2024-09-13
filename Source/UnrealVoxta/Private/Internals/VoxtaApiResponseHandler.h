// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaDefines.h"

class FSignalRValue;
struct IServerResponseBase;
struct ServerResponseWelcome;
struct ServerResponseCharacterList;
struct ServerResponseCharacterLoaded;
struct ServerResponseChatStarted;
struct ServerResponseChatMessageStart;
struct ServerResponseChatMessageChunk;
struct ServerResponseChatMessageEnd;
struct ServerResponseChatMessageCancelled;
struct ServerResponseChatUpdate;
struct ServerResponseSpeechTranscription;

/**
 * VoxtaApiResponseHandler
 * Helper class that provides utility to deserialize VoxtaServer responses that are received via the SignalR socket.
 *
 * Note: All methods and fields must be const (immutable), as this class should remain stateless.
 */
class VoxtaApiResponseHandler
{
#pragma region public API
public:
	/** A collection of message types to ignore. */
	const TSet<FString> ignoredMessageTypes{
		API_STRING("chatStarting"),
		API_STRING("chatLoadingMessage"),
		API_STRING("chatsSessionsUpdated"),
		API_STRING("contextUpdated"),
		API_STRING("replyGenerating"),
		API_STRING("chatFlow"),
		API_STRING("speechRecognitionStart"),
		API_STRING("recordingRequest"),
		API_STRING("recordingStatus"),
		API_STRING("speechPlaybackComplete"),
		API_STRING("memoryUpdated")
	};

	/**
	 * Internal helper class to deserialize a reponse from the VoxtaServer into the corresponding data struct.
	 *
	 * @param serverResponseData The raw data received from the SignalR message.
	 *
	 * @return The uniqueptr to the deserialized object, which derives from IServerResponseBase.
	 */
	TUniquePtr<IServerResponseBase> GetResponseData(
		const TMap<FString, FSignalRValue>& serverResponseData) const;
#pragma endregion

#pragma region VoxtaServer response deserialize handlers
private:
	TUniquePtr<ServerResponseWelcome> GetWelcomeResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseCharacterList> GetCharacterListLoadedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseCharacterLoaded> GetCharacterLoadedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatStarted> GetChatStartedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatMessageStart> GetReplyStartReponseResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatMessageChunk> GetReplyChunkReponseResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatMessageEnd> GetReplyEndReponseResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatMessageCancelled> GetReplyCancelledResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatUpdate> GetChatUpdateResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseSpeechTranscription> GetSpeechRecognitionPartial(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseSpeechTranscription> GetSpeechRecognitionEnd(
		const TMap<FString, FSignalRValue>& serverResponseData) const;
#pragma endregion
};
