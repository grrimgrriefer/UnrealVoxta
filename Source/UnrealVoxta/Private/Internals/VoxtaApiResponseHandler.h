// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaDefines.h"

class FSignalRValue;
struct ServerResponseBase;
struct ServerResponseWelcome;
struct ServerResponseCharacterList;
//struct ServerResponseCharacterLoaded;
struct ServerResponseChatStarted;
struct ServerResponseChatMessageStart;
struct ServerResponseChatMessageChunk;
struct ServerResponseChatMessageEnd;
struct ServerResponseChatMessageCancelled;
struct ServerResponseChatUpdate;
struct ServerResponseSpeechTranscription;
struct ServerResponseError;

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
	/** A collection of message types received from VoxtaServer that are considered safe to ignore. */
	const TSet<FString> IGNORED_MESSAGE_TYPES{
		EASY_STRING("chatStarting"),
		EASY_STRING("chatLoadingMessage"),
		EASY_STRING("chatsSessionsUpdated"),
		EASY_STRING("contextUpdated"),
		EASY_STRING("replyGenerating"),
		EASY_STRING("chatFlow"),
		EASY_STRING("speechRecognitionStart"),
		EASY_STRING("recordingRequest"),
		EASY_STRING("recordingStatus"),
		EASY_STRING("speechPlaybackComplete"),
		EASY_STRING("memoryUpdated")
	};

	/**
	 * Internal helper class to deserialize a reponse from the VoxtaServer into the corresponding data struct.
	 *
	 * @param serverResponseData The raw data received from the SignalR message.
	 *
	 * @return The uniqueptr to the deserialized object, which derives from ServerResponseBase.
	 */
	TUniquePtr<ServerResponseBase> GetResponseData(
		const TMap<FString, FSignalRValue>& serverResponseData) const;
#pragma endregion

#pragma region VoxtaServer response deserialize handlers
private:
	/** ServerResponseWelcome override of the generic GetResponseData */
	TUniquePtr<ServerResponseWelcome> GetWelcomeResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	/** ServerResponseCharacterList override of the generic GetResponseData */
	TUniquePtr<ServerResponseCharacterList> GetCharacterListLoadedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	//	/** ServerResponseCharacterLoaded override of the generic GetResponseData */
	//	TUniquePtr<ServerResponseCharacterLoaded> GetCharacterLoadedResponse(
	//		const TMap<FString, FSignalRValue>& serverResponseData) const;

		/** ServerResponseChatStarted override of the generic GetResponseData */
	TUniquePtr<ServerResponseChatStarted> GetChatStartedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	/** ServerResponseChatMessageStart override of the generic GetResponseData */
	TUniquePtr<ServerResponseChatMessageStart> GetReplyStartReponseResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	/** ServerResponseChatMessageChunk override of the generic GetResponseData */
	TUniquePtr<ServerResponseChatMessageChunk> GetReplyChunkReponseResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	/** ServerResponseChatMessageEnd override of the generic GetResponseData */
	TUniquePtr<ServerResponseChatMessageEnd> GetReplyEndReponseResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	/** ServerResponseChatMessageCancelled override of the generic GetResponseData */
	TUniquePtr<ServerResponseChatMessageCancelled> GetReplyCancelledResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	/** ServerResponseChatUpdate override of the generic GetResponseData */
	TUniquePtr<ServerResponseChatUpdate> GetChatUpdateResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	/** ServerResponseSpeechTranscription (partial) override of the generic GetResponseData */
	TUniquePtr<ServerResponseSpeechTranscription> GetSpeechRecognitionPartial(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	/** ServerResponseSpeechTranscription (end) override of the generic GetResponseData */
	TUniquePtr<ServerResponseSpeechTranscription> GetSpeechRecognitionEnd(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	/** ServerResponseError override of the generic GetResponseData */
	TUniquePtr<ServerResponseError> GetErrorResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;
#pragma endregion
};
