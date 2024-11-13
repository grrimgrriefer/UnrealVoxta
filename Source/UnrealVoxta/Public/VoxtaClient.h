// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaData/Public/VoxtaClientState.h"
#include "VoxtaClient.generated.h"

class FSignalRValue;
class IHubConnection;
class FSignalRInvokeResult;
class UVoxtaAudioInput;
class Audio2FaceRESTHandler;
class UVoxtaAudioPlayback;
class VoxtaLogger;
class VoxtaApiRequestHandler;
class VoxtaApiResponseHandler;
struct ServerResponseBase;
struct ServerResponseError;
struct ServerResponseChatMessageBase;
struct ServerResponseWelcome;
struct ServerResponseCharacterList;
struct ServerResponseCharacterLoaded;
struct ServerResponseChatStarted;
struct ServerResponseChatUpdate;
struct ServerResponseSpeechTranscription;
struct FAiCharData;
struct FUserCharData;
struct FBaseCharData;
struct FChatSession;

/**
 * UVoxtaClient
 * Main public-facing class, contains the stateful client for all Voxta utility.
 * Provides a simple singleton-like API for any external UI / Blueprints / other modules.
 */
UCLASS(DisplayName = "Voxta Client", Category = "Voxta")
class UNREALVOXTA_API UVoxtaClient : public UGameInstanceSubsystem
{
	GENERATED_BODY()

#pragma region delegate declarations
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientStateChanged, VoxtaClientState, newState);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharacterRegistered, const FAiCharData&, charData);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoxtaClientCharMessageAdded, const FBaseCharData&, sender, const FChatMessage&, message);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharMessageRemoved, const FChatMessage&, message);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientSpeechTranscribed, const FString&, message);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientChatSessionStarted, const FChatSession&, chatSession);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoxtaClientAudioPlaybackRegistered, const UVoxtaAudioPlayback*, playbackHandler, const FString&, characterId);

	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientStateChangedNative, VoxtaClientState);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharacterRegisteredNative, const FAiCharData&);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FVoxtaClientCharMessageAddedNative, const FBaseCharData&, const FChatMessage&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharMessageRemovedNative, const FChatMessage&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientSpeechTranscribedNative, const FString&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientChatSessionStartedNative, const FChatSession&);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FVoxtaClientAudioPlaybackRegisteredNative, const UVoxtaAudioPlayback*, const FString&);

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

	/**  Event fired when a playbackHandler has registered itself & claimed audioplayback for that character. */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaClientAudioPlaybackRegistered VoxtaClientAudioPlaybackRegisteredEvent;
	/** Static Event variation of VoxtaClientChatSessionStartedEvent */
	FVoxtaClientAudioPlaybackRegisteredNative VoxtaClientAudioPlaybackRegisteredEventNative;

#pragma endregion

#pragma region UGameInstanceSubsystem overrides
public:
	/** Initialization of the instance of the system */
	virtual void Initialize(FSubsystemCollectionBase& collection) override;
	/** Deinitialization of the instance of the system */
	virtual void Deinitialize() override;
#pragma endregion

#pragma region public API
public:
	/**
	 * Main initializer for the VoxtaClient.
	 * This will start the SignalR connection to the hub and begin listening to server responses.
	 *
	 * Note: There is no restart functionality atm, so this can only be called for a cold-start.
	 *
	 * @param ipv4Address The ipv4 address where the Voxta SignalR hub is hosted.
	 * @param port The HTTP port used for the Voxta SignalR hub.
	 *
	 * TODO: Add restart option
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void StartConnection(const FString& ipv4Address, int port);

	/**
	 * Stops the SignalR connection and terminates the SignalR connection.
	 *
	 * Note: There is no restart functionality at the time being, so only disconnect when you intend to
	 * fully stop using Voxta for the remainder of the play session.
	 *
	 * @param silent Set to true if you want to disconnect without sending a state-change notification.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void Disconnect(bool silent = false);

	/**
	 * Tell the server to initiate a chat session with the character of the provided ID.
	 *
	 * Note: The id must match the id of an already registered character in the client.
	 *
	 * @param charId The charID of the character that you want to load.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void StartChatWithCharacter(const FString& charId);

	/**
	 * Inform the server that the user has said something.
	 *
	 * Note: Autoreply is enabled by default so this will always trigger a reponse from the AI character.
	 *
	 * @param inputText The text that should be considered user input in the conversation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void SendUserInput(const FString& inputText);

	/**
	 * Inform the server that the audioplayback is complete.
	 * Is requird to enable speech recognition on the serverside, to whatever is sent via the audio socket.
	 *
	 * Note: You should only manually call this when you're using 'custom lipsync'.
	 *
	 * @param messageId The ID of the message that has completed the playback on the client.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void NotifyAudioPlaybackComplete(const FString& messageId);

	/**
	 * Register the playback handler for this specific character, this is needed as we need to know if we want to wait
	 * for the audio playback to be completed, or if there's no audio playback and we can just skip it.
	 *
	 * @param characterId The VoxtaServer assigned id of the character that is being registered for.
	 * @param UVoxtaAudioPlayback The audioPlayback component for the specified characterId.
	 *
	 * @return True if the character was registered successfully (no duplicate playback for the same id)
	 */
	bool TryRegisterPlaybackHandler(const FString& characterId, TWeakObjectPtr<UVoxtaAudioPlayback> UVoxtaAudioPlayback);

	/**
	 * Remove the weakPointer to the audioPlayback that was registered for the specified characterId.
	 *
	 * @param characterId The character for which we will remove the weakPointer to whatever audioplayback was
	 * registered for it
	 */
	bool TryUnregisterPlaybackHandler(const FString& characterId);

	/** @return The ipv4 address where this client expects the Voxta server to be hosted. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	const FString& GetServerAddress() const;

	/** @return The HTTP port where this client expects the Voxta server to be hosted. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	int GetServerPort() const;

	/** @return An pointer to the VoxtaAudioInput handler. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	UVoxtaAudioInput* GetVoiceInputHandler() const;

	/** @return The current state of the VoxtaClient internal. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	VoxtaClientState GetCurrentState() const;

	/** @return The current user's ID, assuming we have an authenticated session. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	FString GetUserId() const;

	/**
	 * Try to retrieve a pointer to the UVoxtaAudioPlayback that has claimed playback for the provided characterId.
	 *
	 * @param characterID The character for which you want to retrieve a pointer to the AudioPlayback to.
	 *
	 * @return An immutable pointer to the UVoxtaAudioPlayback component, nullptr if it doesn't exist.
	 */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	const UVoxtaAudioPlayback* GetRegisteredAudioPlaybackHandlerForID(const FString& characterId) const;

	/** @return An immutable pointer to the ChatSession. */
	const FChatSession* GetChatSession() const;

	/** @return An reference to the A2F handler instance, should probably be moved elsewhere, idk yet. */
	Audio2FaceRESTHandler* GetA2FHandler() const;
#pragma endregion

#pragma region data
private:
	const FString SEND_MESSAGE_EVENT_NAME = TEXT("SendMessage");
	const FString RECEIVE_MESSAGE_EVENT_NAME = TEXT("ReceiveMessage");
	const FString LOCALHOST = TEXT("localhost");

	UPROPERTY()
	UVoxtaAudioInput* m_voiceInput;

	TSharedPtr<VoxtaLogger> m_logUtility;
	TSharedPtr<VoxtaApiRequestHandler> m_voxtaRequestApi;
	TSharedPtr<VoxtaApiResponseHandler> m_voxtaResponseApi;
	TSharedPtr<IHubConnection> m_hub;
	TSharedPtr<Audio2FaceRESTHandler> m_A2FHandler;

	TUniquePtr<FUserCharData> m_userData;
	TArray<TUniquePtr<const FAiCharData>> m_characterList;
	TMap<FString, TWeakObjectPtr<UVoxtaAudioPlayback>> m_registeredCharacterPlaybackHandlers;
	TUniquePtr<FChatSession> m_chatSession;
	FString m_hostAddress;
	uint16 m_hostPort;
	VoxtaClientState m_currentState = VoxtaClientState::Disconnected;
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
	 * the message sucessfully. This response does NOT contain any new information, as those are sent
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
		bool (UVoxtaClient::* handler)(const T&));

#pragma region VoxtaServer response handlers
private:
	/** Main response helper, will redirect to the appropriate version with the deserialized data. */
	bool HandleResponse(const TMap<FString, FSignalRValue>& responseData);
	/** Takes care of ServerResponseWelcome responses. */
	bool HandleWelcomeResponse(const ServerResponseWelcome& response);
	/** Takes care of ServerResponseCharacterList responses. */
	bool HandleCharacterListResponse(const ServerResponseCharacterList& response);
	/** Takes care of ServerResponseCharacterLoaded responses. */
	bool HandleCharacterLoadedResponse(const ServerResponseCharacterLoaded& response);
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
#pragma endregion

	/**
	 * Fetch the immutable UniquePtr to the Ai Character data struct that matches the given charId.
	 *
	 * @param charId The Id of the Ai Character that you want to retrieve.
	 *
	 * @return An immutable UniquePtr to the immutable Ai Character data struct, or nullptr if it was not found.
	 */
	const TUniquePtr<const FAiCharData>* GetAiCharacterDataById(const FString& charId) const;

	/**
	 * Fetches a raw pointer to the ChatMessage that maches the id given in the parameters.
	 *
	 * Note: The text & audio in this data is not guarenteed to be complete. Be aware that only after the
	 * id has been broadcasted by VoxtaClientCharMessageAddedEvent that the message is considered final.
	 *
	 * @param messageId The id of the chatmessage you want to retrieve.
	 *
	 * @return A raw pointer to the chatmessage, or nullptr if it was not found.
	 */
	FChatMessage* GetChatMessageById(const FString& messageId) const;

	/**
	 * Update the internal 'current state' to a new state.
	 *
	 * Note: Calling this will trigger the VoxtaClientStateChangedEvent immediately after changing the value.
	 *
	 * @param newState The new state that can be considered to be active in the client.
	 */
	void SetState(VoxtaClientState newState);
#pragma endregion
};
