// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VoxtaData/Public/VoxtaClientState.h"
#include "VoxtaDefines.h"
#include "UserCharData.h"
#include "VoxtaClient.generated.h"

class FSignalRValue;
class IHubConnection;
class FSignalRInvokeResult;
class UVoxtaAudioInput;
class Audio2FaceRESTHandler;
class UVoxtaAudioPlayback;
class AVoxtaGlobalAudioPlaybackHolder;
class VoxtaLogger;
class VoxtaApiRequestHandler;
class VoxtaApiResponseHandler;
class TexturesCacheHandler;
struct ServerResponseBase;
struct ServerResponseError;
struct ServerResponseChatMessageBase;
struct ServerResponseWelcome;
struct ServerResponseCharacterList;
struct ServerResponseChatStarted;
struct ServerResponseChatUpdate;
struct ServerResponseSpeechTranscription;
struct ServerResponseContextUpdated;
struct ServerResponseChatClosed;
struct ServerResponseChatSessionError;
struct FAiCharData;
struct FBaseCharData;
struct FChatSession;
struct FChatMessage;
struct FVoxtaVersionData;

/**
 * UVoxtaClient
 * Main public-facing subsystem for Voxta integration. Manages the stateful connection to the VoxtaServer,
 * handles chat session lifecycle, audio input/output, character and message management, and event broadcasting.
 * Provides a singleton-like API for Blueprints and C++ to interact with Voxta features.
 *
 * Use GetWorld()->GetGameInstance()->GetSubsystem<UVoxtaClient>() to access the instance.
 */
UCLASS(DisplayName = "Voxta Client", Category = "Voxta")
class UNREALVOXTA_API UVoxtaClient : public UGameInstanceSubsystem
{
	GENERATED_BODY()

#pragma region delegate declarations
public:
	/** Delegate fired when the VoxtaClient state changes. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientStateChanged, VoxtaClientState, newState);
	/** Delegate fired when a character is registered. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharacterRegistered, const FAiCharData&, charData);
	/** Delegate fired when a chat message is added. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoxtaClientCharMessageAdded, const FBaseCharData&, sender, const FChatMessage&, message);
	/** Delegate fired when a chat message is removed. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharMessageRemoved, const FChatMessage&, message);
	/** Delegate fired when speech is transcribed. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientSpeechTranscribed, const FString&, message);
	/** Delegate fired when a chat session starts. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientChatSessionStarted, const FChatSession&, chatSession);
	/** Delegate fired when a chat session stops. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientChatSessionStopped, const FChatSession&, chatSession);
	/** Delegate fired when an audio playback handler is registered. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoxtaClientAudioPlaybackRegistered, const UVoxtaAudioPlayback*, playbackHandler, const FGuid&, characterId);

	/** Native C++ delegates for the above events. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientStateChangedNative, VoxtaClientState);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharacterRegisteredNative, const FAiCharData&);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FVoxtaClientCharMessageAddedNative, const FBaseCharData&, const FChatMessage&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharMessageRemovedNative, const FChatMessage&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientSpeechTranscribedNative, const FString&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientChatSessionStartedNative, const FChatSession&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientChatSessionStoppedNative, const FChatSession&);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FVoxtaClientAudioPlaybackRegisteredNative, const UVoxtaAudioPlayback*, const FGuid&);
#pragma endregion

#pragma region events
public:
	/** Event fired when the internal VoxtaClient has finished transitioning to a different state. */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaClientStateChanged VoxtaClientStateChangedEvent;
	/** Static Event variation of VoxtaClientStateChangedEvent */
	FVoxtaClientStateChangedNative VoxtaClientStateChangedEventNative;

	/** Event fired when the internal VoxtaClient has loaded the metadata of an AiCharData. */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaClientCharacterRegistered VoxtaClientCharacterRegisteredEvent;
	/** Static Event variation of VoxtaClientCharacterRegisteredEvent */
	FVoxtaClientCharacterRegisteredNative VoxtaClientCharacterRegisteredEventNative;

	/**
	 * Event fired when the VoxtaClient is notified by the server that a message is added.
	 *
	 * Note: This is triggered for both AI and user messages!
	 */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaClientCharMessageAdded VoxtaClientCharMessageAddedEvent;
	/** Static Event variation of VoxtaClientCharMessageAddedEvent */
	FVoxtaClientCharMessageAddedNative VoxtaClientCharMessageAddedEventNative;

	/** Event fired when the server has notified the client that a message has been removed. */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaClientCharMessageRemoved VoxtaClientCharMessageRemovedEvent;
	/** Static Event variation of VoxtaClientCharMessageRemovedEvent */
	FVoxtaClientCharMessageRemovedNative VoxtaClientCharMessageRemovedEventNative;

	/** Event fired when the server is in progress of transcribing speech, it contains the current version of the transcription. */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaClientSpeechTranscribed VoxtaClientSpeechTranscribedPartialEvent;
	/** Static Event variation of VoxtaClientSpeechTranscribedPartialEvent */
	FVoxtaClientSpeechTranscribedNative VoxtaClientSpeechTranscribedPartialEventNative;

	/** Event fired when the server has finished transcribing speech, it contains the final version of whatever the user said. */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaClientSpeechTranscribed VoxtaClientSpeechTranscribedCompleteEvent;
	/** Static Event variation of VoxtaClientSpeechTranscribedCompleteEvent */
	FVoxtaClientSpeechTranscribedNative VoxtaClientSpeechTranscribedCompleteEventNative;

	/**
	 * Event fired when the chat session has begun.
	 *
	 * Note: This is triggered right after the configuration is set, and before the first initial AI message is received.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaClientChatSessionStarted VoxtaClientChatSessionStartedEvent;
	/** Static Event variation of VoxtaClientChatSessionStartedEvent */
	FVoxtaClientChatSessionStartedNative VoxtaClientChatSessionStartedEventNative;

	/**
	 * Event fired when the chat session has ended.
	 *
	 * Note: The chat object provided in the parameters will be destroyed immediatly after this call.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaClientChatSessionStopped VoxtaClientChatSessionStoppedEvent;
	/** Static Event variation of VoxtaClientChatSessionStoppedEvent */
	FVoxtaClientChatSessionStoppedNative VoxtaClientChatSessionStoppedEventNative;

	/**  Event fired when a playbackHandler has registered itself & claimed audioplayback for that character. */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaClientAudioPlaybackRegistered VoxtaClientAudioPlaybackRegisteredEvent;
	/** Static Event variation of VoxtaClientAudioPlaybackRegisteredEvent */
	FVoxtaClientAudioPlaybackRegisteredNative VoxtaClientAudioPlaybackRegisteredEventNative;
#pragma endregion

#pragma region UGameInstanceSubsystem overrides
public:
	/** Initialization of the instance of the system. Called when the subsystem is created. */
	virtual void Initialize(FSubsystemCollectionBase& collection) override;
	/** Deinitialization of the instance of the system. Called when the subsystem is destroyed. */
	virtual void Deinitialize() override;
#pragma endregion

#pragma region public API
public:
	/**
	 * Start the VoxtaClient and connect to the Voxta SignalR hub.
	 * Only supports cold-start; restart is not supported.
	 *
	 * @param ipv4Address The IPv4 address or hostname of the VoxtaServer.
	 * @param port The HTTP port of the VoxtaServer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void StartConnection(const FString& ipv4Address, int port);

	/**
	 * Disconnect from the VoxtaServer and clean up all resources.
	 * Only call if you intend to stop using Voxta for the rest of the session.
	 *
	 * @param silent If true, do not broadcast a notification for the state-change .
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void Disconnect(bool silent = false);

	/**
	 * Tell the server to initiate a chat session with the character of the provided ID.
	 *
	 * Note: The id must match the id of an already registered character in the client.
	 *
	 * @param charId The character's unique ID.
	 * @param context Optional context string for the chat.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void StartChatWithCharacter(const FGuid& charId, const FString& context = TEXT(""));

	/**
	 * Enable or disable the global audio fallback handler (for characters without a specific handler).
	 * @param newState True to enable, false to disable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void SetGlobalAudioFallbackEnabled(bool newState);

	/** @return True if the global audio fallback is active. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	bool IsGlobalAudioFallbackActive() const;

	/**
	 * Tell the server to stop the ongoing chat session and clean up the relevant dependencies.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void StopActiveChat();

	/**
	 * Update the context of the current chat session.
	 * @param newContext The new context string to send to the server.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void UpdateChatContext(const FString& newContext);

	/**
	 * Send user input text to the server as part of the current chat session.
	 * Triggers an AI reply if generateReply is true.
	 *
	 * @param inputText The user's input text.
	 * @param generateReply Whether to trigger an AI reply.
	 * @param characterActionInference Whether to enable character action inference.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void SendUserInput(const FString& inputText, bool generateReply = true, bool characterActionInference = false);

	/** @return The ipv4 address where this client expects the Voxta server to be hosted. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	const FString& GetServerAddress() const;

	/** @return The HTTP port where this client expects the Voxta server to be hosted. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	int GetServerPort() const;

	/** @return An pointer to the VoxtaAudioInput handler, for microphone capture and streaming. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	UVoxtaAudioInput* GetVoiceInputHandler() const;

	/** @return The current VoxtaClient state. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	VoxtaClientState GetCurrentState() const;

	/** @return The current user's ID, or an invalid Guid if not authenticated. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	FGuid GetUserId() const;

	/**
	 * Get the browser URL for a given AI character.
	 * @param aiCharacterId The character's unique ID.
	 * @return The browser URL as a string.
	 */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	FString GetBrowserUrlForCharacter(const FGuid& aiCharacterId) const;

	/** @return The main assistant's unique ID. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	FGuid GetMainAssistantId() const;

	/**
	 * Asynchronously fetch and cache the thumbnail for a character.
	 * @param baseCharacterId The character's unique ID.
	 * @param onThumbnailFetched Delegate to call when the thumbnail is fetched.
	 */
	void TryFetchAndCacheCharacterThumbnail(const FGuid& baseCharacterId, FDownloadedTextureDelegateNative onThumbnailFetched);

	/**
	 * Try to retrieve a pointer to the UVoxtaAudioPlayback that has claimed playback for the provided characterId.
	 *
	 * @param characterID The character for which you want to retrieve a pointer to the AudioPlayback to.
	 *
	 * @return An immutable pointer to the UVoxtaAudioPlayback component, or nullptr if it doesn't exist.
	 */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	const UVoxtaAudioPlayback* GetRegisteredAudioPlaybackHandlerForID(const FGuid& characterId) const;

	/** @return An copy of every available AIcharacter data. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	TArray<FAiCharData> GetAvailableAiCharactersCopy() const;

	/**
	 * Register the playback handler for this specific character, this is needed as we need to know if we want to wait
	 * for the audio playback to be completed, or if there's no audio playback and we can just skip it.
	 *
	 * @param characterId The VoxtaServer assigned id of the character that is being registered for.
	 * @param playbackHandler The audioPlayback component for the specified characterId.
	 *
	 * @return True if the character was registered successfully, false if duplicate or invalid.
	 */
	bool TryRegisterPlaybackHandler(const FGuid& characterId, TWeakObjectPtr<UVoxtaAudioPlayback> playbackHandler);

	/**
	 * Unregister the audio playback handler for a character.
	 *
	 * @param characterId The character for which we will remove the weakPointer to whatever audioplayback was registered for it.
	 *
	 * @return True if unregistered, false if not found.
	 */
	bool TryUnregisterPlaybackHandler(const FGuid& characterId);

	/** @return A copy of the current chat session. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	FChatSession GetChatSessionCopy() const;

	/** @return A copy of the server version data. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	FVoxtaVersionData GetServerVersionCopy() const;

	/** @return True if the API version matches the server. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	bool IsMatchingAPIVersion() const;

	/** @return An immutable pointer to the current chat session, or nullptr if no chat is active. */
	const FChatSession* GetChatSession() const;

	/** @return Get a weak pointer to the Audio2Face REST handler. Should probably be moved elsewhere, idk yet. */
	TWeakPtr<Audio2FaceRESTHandler> GetA2FHandler() const;

	/**
	 * Enable or disable log censoring for sensitive logs.
	 * @param isCensorActive True to enable censoring, false to disable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void SetCensoredLogs(bool isCensorActive);

	/** @return True if log censoring is active. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	bool IsLogCensorActive() const;
#pragma endregion

#pragma region data
private:
	const FString SEND_MESSAGE_EVENT_NAME = TEXT("SendMessage");
	const FString RECEIVE_MESSAGE_EVENT_NAME = TEXT("ReceiveMessage");

	UPROPERTY()
	UVoxtaAudioInput* m_voiceInput;

	UPROPERTY()
	AVoxtaGlobalAudioPlaybackHolder* m_globalAudioPlaybackComp;
	FDelegateHandle globalAudioPlaybackHandle;
	bool m_enableGlobalAudioFallback = true;

	TSharedPtr<VoxtaLogger> m_logUtility;
	TSharedPtr<IHubConnection> m_hub;
	TSharedPtr<Audio2FaceRESTHandler> m_A2FHandler;
	TSharedPtr<TexturesCacheHandler> m_texturesCacheHandler;

	VoxtaClientState m_currentState = VoxtaClientState::Disconnected;
	TUniquePtr<FUserCharData> m_userData;
	FGuid m_mainAssistantId;
	FString m_hostAddress;
	uint16 m_hostPort;

	TUniquePtr<FVoxtaVersionData> m_voxtaVersionData;
	TUniquePtr<FChatSession> m_chatSession;
	TArray<TUniquePtr<const FAiCharData>> m_characterList;
	TMap<FGuid, TWeakObjectPtr<UVoxtaAudioPlayback>> m_registeredCharacterAudioPlaybackComps;
	TMap<FGuid, FDelegateHandle> m_audioPlaybackHandles;
#pragma endregion

#pragma region private API
private:
	/** Register internal listeners to the event triggers of the SignalR hub connection. */
	void StartListeningToServer();

#pragma region IHubConnection listeners
private:
	/** Called when a new message was received via the connection. */
	void OnReceivedMessage(const TArray<FSignalRValue>& arguments);
	/** Called when a connection has been established successfully. */
	void OnConnected();
	/** Called when a connection could not be established. */
	void OnConnectionError(const FString& error);
	/** Called when a web socket connection has been closed. */
	void OnClosed();
#pragma endregion

	/**
	 * Send a SignalR formatted message to the VoxtaServer.
	 * This also registers the OnMessageSent to be called when receiving the server response.
	 *
	 * @param message The SignalR formatted message to be sent to the server.
	 */
	void SendMessageToServer(const FSignalRValue& message);

	/**
	 * Listener to the reponse from the Voxta server.
	 *
	 * Note: This reponse just notifies the internal VoxtaClient that the server received
	 * the message Successfully. This response does NOT contain any new information, as those are sent
	 * via the OnReceivedMessage function.
	 *
	 * @param deliveryReceiptThe receipt of delivery, given to us by the Server. This should not contain any errors.
	 */
	void OnMessageSent(const FSignalRInvokeResult& deliveryReceipt);

	/**
	 * Template helper function to call the appropriate response handler,
	 * also takes care of casting to the derived type.
	 *
	 * @tparam T The target type to cast towards to, should derive from ServerResponseBase.
	 * @param response The raw reponse object.
	 * @param message The message that will be logged.
	 * @param handler The function responsible for handling the derived object.
	 *
	 * @return True if the response was handled correctly, false otherwise.
	 */
	template<typename T>
	bool HandleResponseHelper(const ServerResponseBase* response, const FString& logMessage,
		bool (UVoxtaClient::* handler)(const T&), bool isSensitive);

#pragma region VoxtaServer response handlers
private:
	/** Main response helper, will redirect to the appropriate version with the deserialized data. */
	bool HandleResponse(const TMap<FString, FSignalRValue>& responseData);
	/** Takes care of ServerResponseWelcome responses. */
	bool HandleWelcomeResponse(const ServerResponseWelcome& response);
	/** Takes care of ServerResponseCharacterList responses. */
	bool HandleCharacterListResponse(const ServerResponseCharacterList& response);
	/** Takes care of ServerResponseChatStarted responses. */
	bool HandleChatStartedResponse(const ServerResponseChatStarted& response);
	/** Takes care of ServerResponseChatMessageBase responses. */
	bool HandleChatMessageResponse(const ServerResponseChatMessageBase& response);
	/** Takes care of ServerResponseChatUpdate responses. */
	bool HandleChatUpdateResponse(const ServerResponseChatUpdate& response);
	/** Takes care of ServerResponseSpeechTranscription responses. */
	bool HandleSpeechTranscriptionResponse(const ServerResponseSpeechTranscription& response);
	/** Takes care of ServerResponseError responses. */
	bool HandleErrorResponse(const ServerResponseError& response);
	/** Takes care of ServerResponseContextUpdated responses. */
	bool HandleContextUpdateResponse(const ServerResponseContextUpdated& response);
	/** Takes care of ServerResponseChatClosed responses. */
	bool HandleChatClosedResponse(const ServerResponseChatClosed& response);
	/** Takes care of ServerResponseChatSessionError responses. */
	bool HandleChatSessionErrorResponse(const ServerResponseChatSessionError& response);
#pragma endregion

	void StopChatInternal();

	/**
	 * Inform the server that the audioplayback is complete.
	 * Is requird to enable speech recognition on the serverside, to whatever is sent via the audio socket.
	 *
	 * @param messageId The ID of the message that has completed the playback on the client.
	 */
	void NotifyAudioPlaybackComplete(const FGuid& messageId);

	/**
	 * Update the internal 'current state' to a new state.
	 *
	 * Note: Calling this will trigger the VoxtaClientStateChangedEvent immediately after changing the value.
	 *
	 * @param newState The new state that can be considered to be active in the client.
	 */
	void SetState(VoxtaClientState newState);

	/**
	 * Fetch the immutable UniquePtr to the Ai Character data struct that matches the given charId.
	 *
	 * @param charId The Id of the Ai Character that you want to retrieve.
	 *
	 * @return An immutable pointer to the UniquePtr to the immutable Ai Character data struct; or nullptr if it was not found.
	 */
	const TUniquePtr<const FAiCharData>* GetAiCharacterDataById(const FGuid& charId) const;

	AVoxtaGlobalAudioPlaybackHolder* GetOrCreateGlobalAudioFallbackInternal();
#pragma endregion
};
