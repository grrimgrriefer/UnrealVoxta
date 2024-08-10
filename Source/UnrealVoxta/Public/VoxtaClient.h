// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxtaUtility/Public/VoxtaLogger.h"
#include "SignalR/Private/HubConnection.h"
#include "VoxtaUtility/Public/VoxtaApiRequestHandler.h"
#include "VoxtaUtility/Public/VoxtaApiResponseHandler.h"
#include "VoxtaData/Public/AiCharData.h"
#include "VoxtaData/Public/ChatMessage.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseBase.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseWelcome.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseCharacterList.h"
#include "VoxtaData/Public/ChatSession.h"
#include "VoxtaData/Public/VoxtaClientState.h"
#include "VoxtaClient.generated.h"

/// <summary>
/// Main public-facing class, containing the stateful client for all Voxta utility.
/// Provides a simple API wrapper for any external UI / Blueprints / other modules.
/// </summary>
UCLASS(HideCategories = (Mobility, Rendering, LOD), ClassGroup = Voxta, meta = (BlueprintSpawnableComponent))
class UNREALVOXTA_API UVoxtaClient : public UActorComponent
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientStateChangedEventCallback, VoxtaClientState, newState);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharacterRegisteredEventCallback, const FAiCharData&, charData);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoxtaClientCharMessageAddedEventCallback, const FCharDataBase&, sender, const FChatMessage&, message);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientCharMessageRemovedEventCallback, const FChatMessage&, message);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientSpeechTranscribedEventCallback, const FString&, message);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaClientChatSessionStartedCallback, const FString&, sessionId);

	/// <summary>
	/// Custom constructor to ensure that gameticks are disabled.
	/// </summary>
	UVoxtaClient();

	///~ Begin UActorComponent overrides.
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	///~ End UActorComponent overrides.

public:
	/// <summary>
	/// Event fired when the internal VoxtaClient has finished transitioning to a different state.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxtaClientStateChangedEventCallback VoxtaClientStateChangedEvent;

	/// <summary>
	/// Event fired when the internal VoxtaClient has loaded the metadata of an AiCharData.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxtaClientCharacterRegisteredEventCallback VoxtaClientCharacterRegisteredEvent;

	/// <summary>
	/// Event fired when the VoxtaClient is notified by the server that a message is added.
	/// Note: this is triggered for both AI and user messages.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxtaClientCharMessageAddedEventCallback VoxtaClientCharMessageAddedEvent;

	/// <summary>
	/// Event fired when the server has notified the client that a message has been removed.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxtaClientCharMessageRemovedEventCallback VoxtaClientCharMessageRemovedEvent;

	/// <summary>
	/// Event fired when the server is in progress of transcribing speech, it contains the current version of the transcription.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxtaClientSpeechTranscribedEventCallback VoxtaClientSpeechTranscribedPartialEvent;

	/// <summary>
	/// Event fired when the server has finished transcribing speech, it contains the final version of whatever the user said.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxtaClientSpeechTranscribedEventCallback VoxtaClientSpeechTranscribedCompleteEvent;

	/// <summary>
	/// Event fired when the chat session has begun. (i.e. the m_chatSession)
	/// Note: Can be used if something needs the chat configuration, before the first initial AI message is received.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxtaClientChatSessionStartedCallback VoxtaClientChatSessionStartedEvent;

	/// <summary>
	/// Main initializer for the VoxtaClient.
	/// This will start the SignalR connection to the hub and begin listening to server responses.
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void StartConnection();

	/// <summary>
	/// Stops the SignalR connection and terminates the SignalR connection.
	/// Note: There is no restart functionality at the time being, so only disconnect when you intend to
	/// fully stop using Voxta for the remainder of the play session.
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void Disconnect(bool silent = false);

	/// <summary>
	/// Tell the server to initiate a chat session with the character of the provided ID.
	/// Note: This will only work if the id matches the id of a registered character in the client.
	/// </summary>
	/// <param name="charId">The FCharDataBase::m_id of the character that you want to load.</param>
	UFUNCTION(BlueprintCallable)
	void StartChatWithCharacter(FString charId);

	/// <summary>
	/// Inform the server that the user has said something.
	/// Note: Autoreply is enabled by default so this will always trigger a reponse from the AI character.
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void SendUserInput(FString inputText);

	/// <summary>
	/// Inform the server that the audioplayback is complete.
	/// Is requird to enable speech recognition on the serverside, to whatever is sent via the audio socket.
	/// </summary>
	/// <param name="messageId">The ID of the message that has completed the playback on the client.</param>
	UFUNCTION()
	void NotifyAudioPlaybackComplete(const FString& messageId);

	const ChatSession* GetChatSession() const;

	UFUNCTION(BlueprintCallable)
	FString GetServerAddress() const;

	UFUNCTION(BlueprintCallable)
	int GetServerPort() const;

private:
	VoxtaLogger m_logUtility;
	TSharedPtr<IHubConnection> m_hub;
	VoxtaApiRequestHandler m_voxtaRequestApi;
	VoxtaApiResponseHandler m_voxtaResponseApi;

	VoxtaClientState m_currentState = VoxtaClientState::Disconnected;
	TUniquePtr<FUserCharData> m_userData;
	TArray<TUniquePtr<const FAiCharData>> m_characterList;
	TUniquePtr<ChatSession> m_chatSession;

	FString m_hostAddress;
	int m_hostPort;

	const FString m_sendMessageEventName = TEXT("SendMessage");
	const FString m_receiveMessageEventName = TEXT("ReceiveMessage");

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
	/// <returns></returns>
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
	void OnMessageSent(const FSignalRInvokeResult& result);

	/// <summary>
	/// Mark the internal VoxtaClient to have finished the transition to a different VoxtaClientState
	/// </summary>
	void SetState(VoxtaClientState newState);

	///~ Begin individual handlers for different VoxtaServer responses.
	bool HandleResponse(const TMap<FString, FSignalRValue>& responseData);
	bool HandleWelcomeResponse(const ServerResponseWelcome& response);
	bool HandleCharacterListResponse(const ServerResponseCharacterList& response);
	bool HandleCharacterLoadedResponse(const ServerResponseCharacterLoaded& response);
	bool HandleChatStartedResponse(const ServerResponseChatStarted& response);
	bool HandleChatMessageResponse(const IServerResponseChatMessageBase& response);
	bool HandleChatUpdateResponse(const ServerResponseChatUpdate& response);
	bool HandleSpeechTranscriptionResponse(const ServerResponseSpeechTranscription& response);
	//~ End individual handlers for different VoxtaServer responses.

	///~ Begin event-listeners for the IHubConnection interface
private:
	void OnReceivedMessage(const TArray<FSignalRValue>& arguments);
	void OnConnected();
	void OnConnectionError(const FString& error);
	void OnClosed();
	//~ End event-listeners for the IHubConnection interface
};
