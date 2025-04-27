// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaDefines.h"

class FSignalRValue;
struct ServerResponseBase;
struct ServerResponseWelcome;
struct ServerResponseCharacterList;
struct ServerResponseContextUpdated;
struct ServerResponseChatStarted;
struct ServerResponseChatMessageStart;
struct ServerResponseChatMessageChunk;
struct ServerResponseChatMessageEnd;
struct ServerResponseChatMessageCancelled;
struct ServerResponseChatUpdate;
struct ServerResponseChatClosed;
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
	static const TSet<FString> IGNORED_MESSAGE_TYPES;

	/**
	 * Internal helper class to deserialize a response from the VoxtaServer into the corresponding data struct.
	 *
	 * @param serverResponseData The raw data received from the SignalR message.
	 *
	 * @return The TUniquePtr to the deserialized object, which derives from ServerResponseBase.
	 */
	static TUniquePtr<ServerResponseBase> GetResponseData(
		const TMap<FString, FSignalRValue>& serverResponseData);
#pragma endregion

#pragma region VoxtaServer response deserialize handlers
private:
	static const TMap<FString, TFunction<TUniquePtr<ServerResponseBase>(const TMap<FString, FSignalRValue>&)>> HANDLERS;

	/** ServerResponseWelcome override of the generic GetResponseData */
	static TUniquePtr<ServerResponseWelcome> GetWelcomeResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** ServerResponseCharacterList override of the generic GetResponseData */
	static TUniquePtr<ServerResponseCharacterList> GetCharacterListLoadedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** ServerResponseContextUpdated override of the generic GetResponseData */
	static TUniquePtr<ServerResponseContextUpdated> GetContextUpdatedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** ServerResponseChatStarted override of the generic GetResponseData */
	static TUniquePtr<ServerResponseChatStarted> GetChatStartedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** ServerResponseChatMessageStart override of the generic GetResponseData */
	static TUniquePtr<ServerResponseChatMessageStart> GetReplyStartResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** ServerResponseChatMessageChunk override of the generic GetResponseData */
	static TUniquePtr<ServerResponseChatMessageChunk> GetReplyChunkResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** ServerResponseChatMessageEnd override of the generic GetResponseData */
	static TUniquePtr<ServerResponseChatMessageEnd> GetReplyEndResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** ServerResponseChatMessageCancelled override of the generic GetResponseData */
	static TUniquePtr<ServerResponseChatMessageCancelled> GetReplyCancelledResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** ServerResponseChatUpdate override of the generic GetResponseData */
	static TUniquePtr<ServerResponseChatUpdate> GetChatUpdateResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** ServerResponseChatClosed override of the generic GetResponseData */
	static TUniquePtr<ServerResponseChatClosed> GetChatClosedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** ServerResponseSpeechTranscription (partial) override of the generic GetResponseData */
	static TUniquePtr<ServerResponseSpeechTranscription> GetSpeechRecognitionPartial(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** ServerResponseSpeechTranscription (end) override of the generic GetResponseData */
	static TUniquePtr<ServerResponseSpeechTranscription> GetSpeechRecognitionEnd(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** ServerResponseError override of the generic GetResponseData */
	static TUniquePtr<ServerResponseError> GetErrorResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Helper function to parse Context data */
	static void ProcessContextData(const TMap<FString, FSignalRValue>& contextMainObject, FString& outContextValue);

	/** Template helper wrapper for mapping without losing type saftey */
	template<typename T>
	static TUniquePtr<ServerResponseBase> WrapHandler(TUniquePtr<T>(*handler)(const TMap<FString, FSignalRValue>&),
		const TMap<FString, FSignalRValue>& data)
	{
		return handler(data);	
	}
#pragma endregion

private:
	static FGuid GetStringAsGuid(const FSignalRValue& input);
	static FGuid GetStringAsGuid(const FString& input);
};
