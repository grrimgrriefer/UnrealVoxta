// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Internals/VoxtaLogger.h"
#include "Internals/VoxtaApiRequestHandler.h"
#include "Internals/VoxtaApiResponseHandler.h"
#include "VoxtaData/Public/VoxtaClientState.h"
#include "VoxtaClient.generated.h"

class FSignalRValue;
class IHubConnection;
class FSignalRInvokeResult;
class UVoxtaAudioInput;
class Audio2FaceRESTHandler;
struct IServerResponseChatMessageBase;
struct ServerResponseWelcome;
struct ServerResponseCharacterList;
struct ServerResponseCharacterLoaded;
struct ServerResponseChatStarted;
struct ServerResponseChatUpdate;
struct ServerResponseSpeechTranscription;
struct FAiCharData;
struct FUserCharData;
struct FCharDataBase;
struct FChatSession;

/**
 * UVoxtaClient
 * Main public-facing class, contains the stateful client for all Voxta utility.
 * Provides a simple singleton-like API for any external UI / Blueprints / other modules.
 */
UCLASS(DisplayName = "Voxta Client")
class UNREALVOXTA_API UVoxtaClient : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
#pragma region delegate declarations
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientStateChanged, VoxtaClientState, newState);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharacterRegistered, const FAiCharData&, charData);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoxtaClientCharMessageAdded, const FCharDataBase&, sender, const FChatMessage&, message);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharMessageRemoved, const FChatMessage&, message);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientSpeechTranscribed, const FString&, message);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientChatSessionStarted, const FChatSession&, chatSession);

	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientStateChangedNative, VoxtaClientState);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharacterRegisteredNative, const FAiCharData&);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FVoxtaClientCharMessageAddedNative, const FCharDataBase&, const FChatMessage&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharMessageRemovedNative, const FChatMessage&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientSpeechTranscribedNative, const FString&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaClientChatSessionStartedNative, const FChatSession&);

#pragma endregion

#pragma region events
	/** Event fired when the internal VoxtaClient has finished transitioning to a different state. */
	UPROPERTY(BlueprintAssignable, Category = Events, meta = (IsBindableEvent = "True"))
	FVoxtaClientStateChanged VoxtaClientStateChangedEvent;
	/** Static Event variation of VoxtaClientStateChangedEvent */
	FVoxtaClientStateChangedNative VoxtaClientStateChangedEventNative;

	/** Event fired when the internal VoxtaClient has loaded the metadata of an AiCharData. */
	UPROPERTY(BlueprintAssignable, Category = Events, meta = (IsBindableEvent = "True"))
	FVoxtaClientCharacterRegistered VoxtaClientCharacterRegisteredEvent;
	/** Static Event variation of VoxtaClientCharacterRegisteredEvent */
	FVoxtaClientCharacterRegisteredNative VoxtaClientCharacterRegisteredEventNative;

	/**
	 * Event fired when the VoxtaClient is notified by the server that a message is added.
	 *
	 * Note: This is triggered for both AI and user messages!
	 */
	UPROPERTY(BlueprintAssignable, Category = Events, meta = (IsBindableEvent = "True"))
	FVoxtaClientCharMessageAdded VoxtaClientCharMessageAddedEvent;
	/** Static Event variation of VoxtaClientCharMessageAddedEvent */
	FVoxtaClientCharMessageAddedNative VoxtaClientCharMessageAddedEventNative;

	/** Event fired when the server has notified the client that a message has been removed. */
	UPROPERTY(BlueprintAssignable, Category = Events, meta = (IsBindableEvent = "True"))
	FVoxtaClientCharMessageRemoved VoxtaClientCharMessageRemovedEvent;
	/** Static Event variation of VoxtaClientCharMessageRemovedEvent */
	FVoxtaClientCharMessageRemovedNative VoxtaClientCharMessageRemovedEventNative;

	/** Event fired when the server is in progress of transcribing speech, it contains the current version of the transcription. */
	UPROPERTY(BlueprintAssignable, Category = Events, meta = (IsBindableEvent = "True"))
	FVoxtaClientSpeechTranscribed VoxtaClientSpeechTranscribedPartialEvent;
	/** Static Event variation of VoxtaClientSpeechTranscribedPartialEvent */
	FVoxtaClientSpeechTranscribedNative VoxtaClientSpeechTranscribedPartialEventNative;

	/** Event fired when the server has finished transcribing speech, it contains the final version of whatever the user said. */
	UPROPERTY(BlueprintAssignable, Category = Events, meta = (IsBindableEvent = "True"))
	FVoxtaClientSpeechTranscribed VoxtaClientSpeechTranscribedCompleteEvent;
	/** Static Event variation of VoxtaClientSpeechTranscribedCompleteEvent */
	FVoxtaClientSpeechTranscribedNative VoxtaClientSpeechTranscribedCompleteEventNative;

	/**
	 * Event fired when the chat session has begun.
	 *
	 * Note: This is triggered right after the configuration is set, and before the first initial AI message is received.
	 */
	UPROPERTY(BlueprintAssignable, Category = Events, meta = (IsBindableEvent = "True"))
	FVoxtaClientChatSessionStarted VoxtaClientChatSessionStartedEvent;
	/** Static Event variation of VoxtaClientChatSessionStartedEvent */
	FVoxtaClientChatSessionStartedNative VoxtaClientChatSessionStartedEventNative;
#pragma endregion

#pragma region UGameInstanceSubsystem overrides
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
#pragma endregion

#pragma region public API
	/**
	 * Main initializer for the VoxtaClient.
	 * This will start the SignalR connection to the hub and begin listening to server responses.
	 *
	 * @param ipv4Address The ipv4 address where the Voxta SignalR hub is hosted.
	 * @param port The HTTP port used for the Voxta SignalR hub.
	 */
	UFUNCTION(BlueprintCallable)
	void StartConnection(const FString& ipv4Address, int port);

	/**
	 * Stops the SignalR connection and terminates the SignalR connection.
	 *
	 * Note: There is no restart functionality at the time being, so only disconnect when you intend to
	 * fully stop using Voxta for the remainder of the play session.
	 *
	 * @param silent Set to true if you want to disconnect without sending a state-change notification.
	 */
	UFUNCTION(BlueprintCallable)
	void Disconnect(bool silent = false);

	/**
	 * Tell the server to initiate a chat session with the character of the provided ID.
	 *
	 * Note: The id must match the id of an already registered character in the client.
	 *
	 * @param charId The charID of the character that you want to load.
	 */
	UFUNCTION(BlueprintCallable)
	void StartChatWithCharacter(FString charId);

	/**
	 * Inform the server that the user has said something.
	 *
	 * Note: Autoreply is enabled by default so this will always trigger a reponse from the AI character.
	 *
	 * @param inputText The text that should be considered user input in the conversation.
	 */
	UFUNCTION(BlueprintCallable)
	void SendUserInput(FString inputText);

	/**
	 * Inform the server that the audioplayback is complete.
	 * Is requird to enable speech recognition on the serverside, to whatever is sent via the audio socket.
	 *
	 * Note: You should only manually call this when you're using 'custom lipsync'.
	 *
	 * @param messageId The ID of the message that has completed the playback on the client.
	 */
	UFUNCTION(BlueprintCallable)
	void NotifyAudioPlaybackComplete(const FString& messageId);

	/** @return The ipv4 address where this client expects the Voxta server to be hosted. */
	UFUNCTION(BlueprintCallable)
	FString GetServerAddress() const;

	/** @return The HTTP port where this client expects the Voxta server to be hosted. */
	UFUNCTION(BlueprintCallable)
	int GetServerPort() const;

	/** @return An pointer to the VoxtaAudioInput handler. */
	UFUNCTION(BlueprintCallable)
	UVoxtaAudioInput* GetVoiceInputHandler() const;

	/** @return An immutable pointer to the ChatSession. */
	const FChatSession* GetChatSession() const;

	/** @return An reference to the A2F handler instance, should probably be moved elsewhere, idk yet. */
	Audio2FaceRESTHandler* GetA2FHandler() const;

#pragma endregion

protected:
	UPROPERTY()
	UVoxtaAudioInput* m_voiceInput;

	TSharedPtr<IHubConnection> m_hub;
	TUniquePtr<Audio2FaceRESTHandler> m_A2FHandler;

private:
	const FString m_sendMessageEventName = TEXT("SendMessage");
	const FString m_receiveMessageEventName = TEXT("ReceiveMessage");

	VoxtaLogger m_logUtility;
	VoxtaApiRequestHandler m_voxtaRequestApi;
	VoxtaApiResponseHandler m_voxtaResponseApi;

#pragma region raw data
	TUniquePtr<FUserCharData> m_userData;
	TArray<TUniquePtr<const FAiCharData>> m_characterList;
	TUniquePtr<FChatSession> m_chatSession;
	FString m_hostAddress;
	uint16 m_hostPort;
	VoxtaClientState m_currentState = VoxtaClientState::Disconnected;
#pragma endregion

#pragma region private API
	/// <summary>
	/// Register internal listeners to the event triggers of the SignalR hub connection.
	/// </summary>
	void StartListeningToServer();

	/// <summary>
	/// Template helper function to call the appropriate response handler, also takes care of casting to the derived type.
	/// </summary>
	/// <typeparam name="T">Derived type, inherits from IServerResponseBase.</typeparam>
	/// <param name="response">The raw reponse object</param>
	/// <param name="message">The message that will be logged.</param>
	/// <param name="handler">The function responsible for handling the derived object.</param>
	/// <returns>True if the response was handled correctly, false otherwise.</returns>
	template<typename T>
	bool HandleResponseHelper(const IServerResponseBase* response, const char* message, bool (UVoxtaClient::* handler)(const T&));

	/// <summary>
	/// Send a SignalR formatted message to the VoxtaServer.
	/// This also registers the OnMessageSent to be called when receiving the server response.
	/// </summary>
	/// <param name="message">The SignalR formatted message to be sent to the server.</param>
	void SendMessageToServer(const FSignalRValue& message);

	/// <summary>
	/// Listener to the reponse from the Voxta server.
	/// Note: This reponse just notifies the internal VoxtaClient that the server received
	/// the message sucessfully. This response does NOT contain any new information, as those are sent
	/// via the OnReceivedMessage function.
	/// </summary>
	/// <param name="deliveryReceipt">The receipt of delivery, given to us by the Server.
	/// should not contain any errors.</param>
	void OnMessageSent(const FSignalRInvokeResult& deliveryReceipt);

	/// <summary>
	///
	/// </summary>
	/// <param name="charId"></param>
	/// <returns></returns>
	const TUniquePtr<const FAiCharData>* GetCharacterDataById(const FString& charId);

	FChatMessage* GetChatMessageById(const FString& messageId);

	/// <summary>
	/// Mark the internal VoxtaClient to have finished the transition to a different VoxtaClientState
	/// </summary>
	void SetState(VoxtaClientState newState);

#pragma region VoxtaServer response handlers
	bool HandleResponse(const TMap<FString, FSignalRValue>& responseData);
	bool HandleWelcomeResponse(const ServerResponseWelcome& response);
	bool HandleCharacterListResponse(const ServerResponseCharacterList& response);
	bool HandleCharacterLoadedResponse(const ServerResponseCharacterLoaded& response);
	bool HandleChatStartedResponse(const ServerResponseChatStarted& response);
	bool HandleChatMessageResponse(const IServerResponseChatMessageBase& response);
	bool HandleChatUpdateResponse(const ServerResponseChatUpdate& response);
	bool HandleSpeechTranscriptionResponse(const ServerResponseSpeechTranscription& response);
#pragma endregion

#pragma region IHubConnection listeners
	void OnReceivedMessage(const TArray<FSignalRValue>& arguments);
	void OnConnected();
	void OnConnectionError(const FString& error);
	void OnClosed();
#pragma endregion

#pragma endregion
};
