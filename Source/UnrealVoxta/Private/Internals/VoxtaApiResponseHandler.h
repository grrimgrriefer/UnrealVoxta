// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

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
struct ServerResponseChatSessionError;
struct ServerResponseConfiguration;

/**
 * VoxtaApiResponseHandler
 * Stateless utility class for deserializing and mapping raw SignalR responses from the VoxtaServer
 * into strongly-typed C++ data structures. Used internally by VoxtaClient to process server messages.
 *
 * All methods are static and the class holds no mutable state.
 */
class VoxtaApiResponseHandler
{
#pragma region public API
public:
	/** 
	 * Set of message types that we could receive from VoxtaServer but are considered safe to ignore and require no handling.
	 */
	static const TSet<FString> IGNORED_MESSAGE_TYPES;

	/**
	 * Deserialize a SignalR response from the VoxtaServer into the corresponding ServerResponseBase-derived struct.
	 *
	 * @param serverResponseData The raw data received from the SignalR message.
	 * @return A TUniquePtr to the deserialized object, or nullptr if deserialization failed.
	 */
	static TUniquePtr<ServerResponseBase> GetResponseData(
		const TMap<FString, FSignalRValue>& serverResponseData);
#pragma endregion

#pragma region VoxtaServer response deserialize handlers
private:
	/**
	 * Map of string to handler function for deserializing each response type.
	 */
	static const TMap<FString, TFunction<TUniquePtr<ServerResponseBase>(const TMap<FString, FSignalRValue>&)>> HANDLERS;

	/** Deserialize a welcome response. */
	static TUniquePtr<ServerResponseWelcome> GetWelcomeResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a character list response. */
	static TUniquePtr<ServerResponseCharacterList> GetCharacterListLoadedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a context updated response. */
	static TUniquePtr<ServerResponseContextUpdated> GetContextUpdatedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a chat started response. */
	static TUniquePtr<ServerResponseChatStarted> GetChatStartedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a chat message start response. */
	static TUniquePtr<ServerResponseChatMessageStart> GetReplyStartResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a chat message chunk response. */
	static TUniquePtr<ServerResponseChatMessageChunk> GetReplyChunkResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a chat message end response. */
	static TUniquePtr<ServerResponseChatMessageEnd> GetReplyEndResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a chat message cancelled response. */
	static TUniquePtr<ServerResponseChatMessageCancelled> GetReplyCancelledResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a chat update response. */
	static TUniquePtr<ServerResponseChatUpdate> GetChatUpdateResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a chat closed response. */
	static TUniquePtr<ServerResponseChatClosed> GetChatClosedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a partial speech transcription response. */
	static TUniquePtr<ServerResponseSpeechTranscription> GetSpeechRecognitionPartial(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a completed speech transcription response. */
	static TUniquePtr<ServerResponseSpeechTranscription> GetSpeechRecognitionEnd(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize an error response. */
	static TUniquePtr<ServerResponseError> GetErrorResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a chatsession specific  error response. */
	static TUniquePtr<ServerResponseChatSessionError> GetChatSessionErrorResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/** Deserialize a configuration response. */
	static TUniquePtr<ServerResponseConfiguration> GetConfigurationResponse(
		const TMap<FString, FSignalRValue>& serverResponseData);

	/**
	 * Extract the context string from a context object in the response.
	 * @param contextMainObject The context object from the response.
	 * @param outContextValue Output parameter for the extracted context string.
	 */
	static void ProcessContextData(const TMap<FString, FSignalRValue>& contextMainObject, FString& outContextValue);

	/**
	 * Helper for mapping handler functions to TUniquePtr<ServerResponseBase> without losing type safety.
	 */
	template<typename T>
	static TUniquePtr<ServerResponseBase> WrapHandler(TUniquePtr<T>(*handler)(const TMap<FString, FSignalRValue>&),
		const TMap<FString, FSignalRValue>& data)
	{
		return handler(data);	
	}
#pragma endregion

private:
	/** @return The SignalR-string parsed as a GUID.  */
	static FGuid GetStringAsGuid(const FSignalRValue& input);

	/** @return The string parsed as a GUID.  */
	static FGuid GetStringAsGuid(const FString& input);
};
