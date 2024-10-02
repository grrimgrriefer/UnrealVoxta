// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaClient.h"
#include "SignalR/Public/SignalRSubsystem.h"
#include "SignalR/Private/HubConnection.h"
#include "VoxtaData/Public/ChatSession.h"
#include "Audio2FaceRESTHandler.h"
#include "VoxtaDefines.h"
#include "Logging/StructuredLog.h"
#include "VoxtaAudioInput.h"
#include "VoxtaAudioPlayback.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseBase.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseWelcome.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseCharacterList.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseCharacterLoaded.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatStarted.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatMessageStart.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatMessageChunk.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatMessageEnd.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatMessageCancelled.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatUpdate.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseSpeechTranscription.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseError.h"

void UVoxtaClient::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);
	m_logUtility.RegisterVoxtaLogger();
	m_voiceInput = NewObject<UVoxtaAudioInput>(this);
	m_A2FHandler = MakeShared<Audio2FaceRESTHandler>();
}

void UVoxtaClient::Deinitialize()
{
	if (m_currentState != VoxtaClientState::Disconnected)
	{
		Disconnect(true);
	}
	Super::Deinitialize();
}

void UVoxtaClient::StartConnection(const FString& ipv4Address, int port)
{
	if (m_currentState != VoxtaClientState::Disconnected)
	{
		UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient is already in state: {0}, ignoring new connection attempt",
			UEnum::GetValueAsString(m_currentState));
		return;
	}

	if (port < 0 || port > MAX_uint16)
	{
		UE_LOGFMT(VoxtaLog, Error, "Port {0} is an impossible number, please double check your settings. "
			"Ignoring connection attempt.", port);
		return;
	}

	m_hostAddress = ipv4Address;
	m_hostPort = port;

	m_hub = GEngine->GetEngineSubsystem<USignalRSubsystem>()->CreateHubConnection(
		FString::Format(*EASY_STRING("http://{0}:{1}/hub"), {
			m_hostAddress,
			m_hostPort
		}));

	StartListeningToServer();
	m_hub->Start();
	SetState(VoxtaClientState::AttemptingToConnect);

	UE_LOGFMT(VoxtaLog, Log, "Starting Voxta client");
}

void UVoxtaClient::Disconnect(bool silent)
{
	if (m_currentState == VoxtaClientState::Disconnected)
	{
		UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient is currently not connected, ignoring disconnect attempt");
		return;
	}
	if (!silent)
	{
		SetState(VoxtaClientState::Terminated);
	}
	m_hub->Stop();
}

void UVoxtaClient::StartChatWithCharacter(const FString& charId)
{
	if (GetAiCharacterDataById(charId) != nullptr)
	{
		SendMessageToServer(m_voxtaRequestApi.GetLoadCharacterRequestData(charId));
		SetState(VoxtaClientState::StartingChat);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Cannot start a chat with characterId {0} as it's not in the current character list.",
			charId);
	}
}

void UVoxtaClient::SendUserInput(const FString& inputText)
{
	if (m_currentState == VoxtaClientState::WaitingForUserReponse)
	{
		SendMessageToServer(m_voxtaRequestApi.GetSendUserMessageData(m_chatSession->GetSessionId(), inputText));
		SetState(VoxtaClientState::GeneratingReply);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Cannot send userInput as the VoxtaServer is currently {0}, "
			"please wait untile it's WaitingForUserResponse.", UEnum::GetValueAsString(m_currentState));
	}
}

void UVoxtaClient::NotifyAudioPlaybackComplete(const FString& messageId)
{
	UE_LOGFMT(VoxtaLog, Log, "Marking audio playback of message {0} complete.", messageId);

	SendMessageToServer(m_voxtaRequestApi.GetNotifyAudioPlaybackCompleteData(m_chatSession->GetSessionId(), messageId));
	SetState(VoxtaClientState::WaitingForUserReponse);
}

const FString& UVoxtaClient::GetServerAddress() const
{
	return m_hostAddress;
}

int UVoxtaClient::GetServerPort() const
{
	return m_hostPort;
}

UVoxtaAudioInput* UVoxtaClient::GetVoiceInputHandler() const
{
	return m_voiceInput;
}

VoxtaClientState UVoxtaClient::GetCurrentState() const
{
	return m_currentState;
}

const UVoxtaAudioPlayback* UVoxtaClient::GetRegisteredAudioPlaybackHandlerForID(const FString& characterId) const
{
	auto currentHandler = m_registeredCharacterPlaybackHandlers.Find(characterId);
	if (currentHandler == nullptr)
	{
		return currentHandler->Get();
	}
	else
	{
		return nullptr;
	}
}

const FChatSession* UVoxtaClient::GetChatSession() const
{
	return m_chatSession.Get();
}

Audio2FaceRESTHandler* UVoxtaClient::GetA2FHandler() const
{
	return m_A2FHandler.Get();
}

bool UVoxtaClient::TryRegisterPlaybackHandler(const FString& characterId,
	TWeakObjectPtr<UVoxtaAudioPlayback> playbackHandler)
{
	if (m_chatSession != nullptr && !m_chatSession->GetActiveServices().Contains(
		VoxtaServiceData::ServiceType::SpeechToText))
	{
		UE_LOGFMT(VoxtaLog, Warning, "You tried to register a Voxta AudioPlayback handler for character {0}, but no "
			"STT service is active on VoxtaServer. (make sure to have it enabled at start, runtime activation not yet "
			"supported...)", characterId);
		return false;
	}

	auto currentHandler = m_registeredCharacterPlaybackHandlers.Find(characterId);
	if (currentHandler == nullptr)
	{
		m_registeredCharacterPlaybackHandlers.Emplace(characterId, playbackHandler);
		UE_LOGFMT(VoxtaLog, Log, "Voxta Audioplayback handler for character: {0} registered successfully.", characterId);

		if (playbackHandler->GetLipSyncType() == LipSyncType::Audio2Face)
		{
			UE_LOGFMT(VoxtaLog, Log, "Voxta Audioplayback handler of character: {0} requires A2F, trying to establish "
				"connection... hold on.", characterId);
			m_A2FHandler->TryInitialize();
		}

		FVoxtaClientAudioPlaybackRegisteredEventNative.Broadcast(playbackHandler.Get(), characterId);
		FVoxtaClientAudioPlaybackRegisteredEvent.Broadcast(playbackHandler.Get(), characterId);
		return true;
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "A Voxta Audioplayback handler for character: {0} already exists. "
			"Multiple audio playback handlers for the same character is not supported... skipping registration.",
			characterId);
		return false;
	}
}

void UVoxtaClient::UnregisterPlaybackHandler(const FString& characterId)
{
	m_registeredCharacterPlaybackHandlers.Remove(characterId);
	UE_LOGFMT(VoxtaLog, Log, "Voxta Audioplayback handler for character: {0} unregistered successfully.", characterId);
}

void UVoxtaClient::StartListeningToServer()
{
	m_hub->On(RECEIVE_MESSAGE_EVENT_NAME).BindUObject(this, &UVoxtaClient::OnReceivedMessage);
	m_hub->OnConnected().AddUObject(this, &UVoxtaClient::OnConnected);
	m_hub->OnConnectionError().AddUObject(this, &UVoxtaClient::OnConnectionError);
	m_hub->OnClosed().AddUObject(this, &UVoxtaClient::OnClosed);
}

void UVoxtaClient::OnReceivedMessage(const TArray<FSignalRValue>& arguments)
{
	/** Wait on the next tick to run the responsehandling on the GameThread,
	 * instead of the background thread of the socket. */
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
		[Self = TWeakObjectPtr<UVoxtaClient>(this), Arguments = arguments] (float deltaTime)
		{
			if (Self != nullptr)
			{
				if (Self->m_currentState == VoxtaClientState::Disconnected ||
					Self->m_currentState == VoxtaClientState::Terminated)
				{
					UE_LOGFMT(VoxtaLog, Warning, "Tried to process a message with the connection already severed, "
						"skipping processing of remaining response data.");
				}
				else if (Arguments.IsEmpty() || Arguments[0].GetType() != FSignalRValue::EValueType::Object)
				{
					UE_LOGFMT(VoxtaLog, Error, "Received invalid message from server.");
				}
				else if (Self->HandleResponse(Arguments[0].AsObject()))
				{
					UE_LOGFMT(VoxtaLog, Log, "VoxtaServer message handled successfully.");
				}
				else
				{
					UE_LOGFMT(VoxtaLog, Warning, "Response handler reported a failure, please check the logs to see "
						"what's wrong. Type: {0}", Arguments[0].AsObject()[EASY_STRING("$type")].AsString());
				}
			}
			// We don't care about else, as that means the 'playmode' is over.
			return false; // Return false to remove the ticker after it runs once
		}));
}

void UVoxtaClient::OnConnected()
{
	/** Wait on the next tick to run the responsehandling on the GameThread,
	 * instead of the background thread of the socket. */
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
		[Self = TWeakObjectPtr<UVoxtaClient>(this)] (float deltatime)
		{
			if (Self != nullptr)
			{
				UE_LOGFMT(VoxtaLog, Log, "VoxtaClient connected successfully");
				Self->SendMessageToServer(Self->m_voxtaRequestApi.GetAuthenticateRequestData());
			}
			// We don't care about else, as that means the 'playmode' is over.
			return false; // Return false to remove the ticker after it runs once
		}));
}

void UVoxtaClient::OnConnectionError(const FString& error)
{
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
		[Self = TWeakObjectPtr<UVoxtaClient>(this), Error = error] (float deltatime)
		{
			if (Self != nullptr)
			{
				UE_LOGFMT(VoxtaLog, Error, "VoxtaClient connection has encountered error: {0}.", Error);
				Self->Disconnect();
			}
			// We don't care about else, as that means the 'playmode' is over.
			return false; // Return false to remove the ticker after it runs once
		}));
}

void UVoxtaClient::OnClosed()
{
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
		[Self = TWeakObjectPtr<UVoxtaClient>(this)] (float deltatime)
		{
			if (Self != nullptr)
			{
				UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient connection has been closed.");
				Self->Disconnect();
			}
			// We don't care about else, as that means the 'playmode' is over.
			return false; // Return false to remove the ticker after it runs once
		}));
}

void UVoxtaClient::SendMessageToServer(const FSignalRValue& message)
{
	m_hub->Invoke(SEND_MESSAGE_EVENT_NAME, message).BindUObject(this, &UVoxtaClient::OnMessageSent);
}

void UVoxtaClient::OnMessageSent(const FSignalRInvokeResult& deliveryReceipt)
{
	if (deliveryReceipt.HasError())
	{
		UE_LOGFMT(VoxtaLog, Error, "Failed to send message due to error: {0}.", deliveryReceipt.GetErrorMessage());
	}
}

template<typename T>
bool UVoxtaClient::HandleResponseHelper(const ServerResponseBase* response, const FString& logMessage,
	bool (UVoxtaClient::* handler)(const T&))
{
	const T* derivedResponse = StaticCast<const T*>(response);
	if (derivedResponse)
	{
		UE_LOGFMT(VoxtaLog, Log, "{0}", logMessage);
		return (this->*handler)(*derivedResponse);
	}
	return false;
}

bool UVoxtaClient::HandleResponse(const TMap<FString, FSignalRValue>& responseData)
{
	FString responseType = responseData[EASY_STRING("$type")].AsString();
	if (m_voxtaResponseApi.IGNORED_MESSAGE_TYPES.Contains(responseType))
	{
		UE_LOGFMT(VoxtaLog, Log, "Ignoring message of type: {0}", responseType);
		return true;
	}

	TUniquePtr<ServerResponseBase> response = m_voxtaResponseApi.GetResponseData(responseData);
	if (!response.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Error, "Failed to deserialize message of type: {0}", responseType);
		return false;
	}

	switch (response->RESPONSE_TYPE)
	{
		using enum ServerResponseType;
		case Welcome:
			return HandleResponseHelper<ServerResponseWelcome>(response.Get(),
				TEXT("Logged in successfully"), &UVoxtaClient::HandleWelcomeResponse);
		case CharacterList:
			return HandleResponseHelper<ServerResponseCharacterList>(response.Get(),
				TEXT("Fetched characters successfully"), &UVoxtaClient::HandleCharacterListResponse);
		case CharacterLoaded:
			return HandleResponseHelper<ServerResponseCharacterLoaded>(response.Get(),
				TEXT("Loaded character successfully"), &UVoxtaClient::HandleCharacterLoadedResponse);
		case ChatStarted:
			return HandleResponseHelper<ServerResponseChatStarted>(response.Get(),
				TEXT("Chat started successfully"), &UVoxtaClient::HandleChatStartedResponse);
		case ChatMessage:
			return HandleResponseHelper<ServerResponseChatMessageBase>(response.Get(),
				TEXT("Chat Message received successfully"), &UVoxtaClient::HandleChatMessageResponse);
		case ChatUpdate:
			return HandleResponseHelper<ServerResponseChatUpdate>(response.Get(),
				TEXT("Chat Update received successfully"), &UVoxtaClient::HandleChatUpdateResponse);
		case SpeechTranscription:
			return HandleResponseHelper<ServerResponseSpeechTranscription>(response.Get(),
				TEXT("Speech transcription update received successfully"),
				&UVoxtaClient::HandleSpeechTranscriptionResponse);
		case Error:
			return HandleResponseHelper<ServerResponseError>(response.Get(),
				TEXT("Error message received successfully"),
				&UVoxtaClient::HandleErrorResponse);
		default:
			UE_LOGFMT(VoxtaLog, Error, "No handler available for type a message of type: {0}", responseType);
			return false;
	}
}

bool UVoxtaClient::HandleWelcomeResponse(const ServerResponseWelcome& response)
{
	m_userData = MakeUnique<FUserCharData>(response.USER_DATA);

	UE_LOGFMT(VoxtaLog, Log, "Authenticated with Voxta Server. Welcome {0}! :D", m_userData->GetName());

	SetState(VoxtaClientState::Authenticated);
	SendMessageToServer(m_voxtaRequestApi.GetLoadCharactersListData());
	return true;
}

bool UVoxtaClient::HandleCharacterListResponse(const ServerResponseCharacterList& response)
{
	m_characterList.Empty();
	for (const FAiCharData charElement : response.CHARACTERS)
	{
		m_characterList.Emplace(MakeUnique<FAiCharData>(charElement));
		VoxtaClientCharacterRegisteredEventNative.Broadcast(charElement);
		VoxtaClientCharacterRegisteredEvent.Broadcast(charElement);
	}
	SetState(VoxtaClientState::Idle);
	return true;
}

bool UVoxtaClient::HandleCharacterLoadedResponse(const ServerResponseCharacterLoaded& response)
{
	const TUniquePtr<const FAiCharData>* character = GetAiCharacterDataById(response.CHARACTER_ID);

	if (character)
	{
		SendMessageToServer(m_voxtaRequestApi.GetStartChatRequestData(character->Get()));
		return true;
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Loaded a character with id: {0} that doesn't exist in the list? "
			"This should never happen..", response.CHARACTER_ID);
		return false;
	}
}

bool UVoxtaClient::HandleChatStartedResponse(const ServerResponseChatStarted& response)
{
	TArray<const FAiCharData*> chatCharacters;
	for (const TUniquePtr<const FAiCharData>& aiChar : m_characterList)
	{
		if (response.CHARACTER_IDS.Contains(aiChar->GetId()))
		{
			chatCharacters.Add(aiChar.Get());
		}
	}

	if (!response.SERVICES.Contains(VoxtaServiceData::ServiceType::SpeechToText))
	{
		UE_LOGFMT(VoxtaLog, Log, "No valid SpeechToText service is active on the server.");
	}
	if (!response.SERVICES.Contains(VoxtaServiceData::ServiceType::TextToSpeech))
	{
		UE_LOGFMT(VoxtaLog, Log, "No valid TextToSpeech service is active on the server.");
	}
	if (!response.SERVICES.Contains(VoxtaServiceData::ServiceType::TextGen))
	{
		UE_LOGFMT(VoxtaLog, Error, "No valid TextGen service is active on the server. We cannot really do anything "
			"without this... aborting creation of chat session.");
		return false;
	}

	m_chatSession = MakeUnique<FChatSession>(chatCharacters, response.CHAT_ID,
		response.SESSION_ID, response.SERVICES);
	VoxtaClientChatSessionStartedEventNative.Broadcast(*m_chatSession.Get());
	VoxtaClientChatSessionStartedEvent.Broadcast(*m_chatSession.Get());
	SetState(VoxtaClientState::GeneratingReply);
	return true;
}

bool UVoxtaClient::HandleChatMessageResponse(const ServerResponseChatMessageBase& response)
{
	if (m_chatSession == nullptr)
	{
		UE_LOGFMT(VoxtaLog, Error, "Received a chat message, but there's no ongoing chat, "
			"a critical service was likely not available.");
		return false;
	}

	TArray<FChatMessage>& messages = m_chatSession->GetChatMessages();
	using enum ServerResponseChatMessageBase::ChatMessageType;
	switch (response.MESSAGE_TYPE)
	{
		case MessageStart:
		{
			const ServerResponseChatMessageStart* derivedResponse =
				StaticCast<const ServerResponseChatMessageStart*>(&response);
			messages.Emplace(FChatMessage(derivedResponse->MESSAGE_ID, derivedResponse->SENDER_ID));

			UE_LOGFMT(VoxtaLog, Log, "Registered start of message with id: {0}", derivedResponse->MESSAGE_ID);
			break;
		}
		case MessageChunk:
		{
			const ServerResponseChatMessageChunk* derivedResponse =
				StaticCast<const ServerResponseChatMessageChunk*>(&response);
			FChatMessage* chatMessage = GetChatMessageById(derivedResponse->MESSAGE_ID);

			if (chatMessage)
			{
				// Add a space before the text when appending it to the previous. As VoxtaServer ends the last sentence
				// of a chunk directly after the period, without an extra space.
				chatMessage->AppendMoreContent(chatMessage->GetTextContent().IsEmpty()
					? derivedResponse->MESSAGE_TEXT
					: " " + derivedResponse->MESSAGE_TEXT, derivedResponse->AUDIO_URL_PATH);
				UE_LOGFMT(VoxtaLog, Log, "Updated message contents of message with id: {0}", derivedResponse->MESSAGE_ID);
			}
			else
			{
				UE_LOGFMT(VoxtaLog, Warning, "Received a messageChunk without already having the start of the message. "
					"messageId: {0} content: {1}", chatMessage->GetMessageId(), chatMessage->GetTextContent());
			}
			break;
		}
		case MessageEnd:
		{
			const ServerResponseChatMessageEnd* derivedResponse =
				StaticCast<const ServerResponseChatMessageEnd*>(&response);
			const FChatMessage* chatMessage = GetChatMessageById(derivedResponse->MESSAGE_ID);

			if (chatMessage)
			{
				const TUniquePtr<const FAiCharData>* character = GetAiCharacterDataById(derivedResponse->SENDER_ID);

				if (character->IsValid())
				{
					UE_LOGFMT(VoxtaLog, Log, "Message with id: {0} marked as complete. Speaker: {1} Contents: {2}",
						derivedResponse->MESSAGE_ID, character->Get()->GetName(), chatMessage->GetTextContent());

					auto playbackHandler = m_registeredCharacterPlaybackHandlers.Find(character->Get()->GetId());
					if (playbackHandler != nullptr)
					{
						SetState(VoxtaClientState::AudioPlayback);
						playbackHandler->Get()->PlaybackMessage(*character->Get(), *chatMessage);
					}
					else
					{
						if (chatMessage->GetAudioUrls().Num() > 0)
						{
							UE_LOGFMT(VoxtaLog, Warning, "Audio data was generate for characterId: {0} but no playback "
								"handler was found. You might want to disable TTS service if you don't want audio playback. "
								"Or add a VoxtaAudioPlayback component if you desire audioplayback. Name: {1} Contents: {2}",
								character->Get()->GetName(), character->Get()->GetName(), chatMessage->GetTextContent());
							NotifyAudioPlaybackComplete(chatMessage->GetMessageId());
						}
						else
						{
							SetState(VoxtaClientState::WaitingForUserReponse);
						}
					}
					VoxtaClientCharMessageAddedEventNative.Broadcast(*character->Get(), *chatMessage);
					VoxtaClientCharMessageAddedEvent.Broadcast(*character->Get(), *chatMessage);
				}
				else
				{
					UE_LOGFMT(VoxtaLog, Warning, "Received a messageEnd for a character that we don't have registered. "
						"senderId: {0} messageId: {1}", derivedResponse->SENDER_ID, chatMessage->GetMessageId());
				}
			}
			else
			{
				UE_LOGFMT(VoxtaLog, Warning, "Received a messageEnd without already having the start of the message. "
					"messageId: {0} content: {1}", chatMessage->GetMessageId(), chatMessage->GetTextContent());
			}
			break;
		}
		case MessageCancelled:
		{
			const ServerResponseChatMessageCancelled* derivedResponse =
				StaticCast<const ServerResponseChatMessageCancelled*>(&response);
			int index = messages.IndexOfByPredicate([derivedResponse] (const FChatMessage& InItem)
				{
					return InItem.GetMessageId() == derivedResponse->MESSAGE_ID;
				});

			UE_LOGFMT(VoxtaLog, Log, "Message with id: {0} marked as cancelled, removing it from the history.",
				derivedResponse->MESSAGE_ID);

			VoxtaClientCharMessageRemovedEventNative.Broadcast(messages[index]);
			VoxtaClientCharMessageRemovedEvent.Broadcast(messages[index]);
			messages.RemoveAt(index);
		}
	}
	return true;
}

bool UVoxtaClient::HandleChatUpdateResponse(const ServerResponseChatUpdate& response)
{
	if (response.SESSION_ID == m_chatSession->GetSessionId())
	{
		if (GetChatMessageById(response.MESSAGE_ID) != nullptr)
		{
			UE_LOGFMT(VoxtaLog, Error, "Recieved a chat update but a message with that id already exists, "
				"let me know if this ever triggers as it has no implementation. Sender: {0} MessageId {1} Content: {2}",
				response.SENDER_ID, response.MESSAGE_ID, response.TEXT_CONTENT);
			return false;
		}
		else
		{
			if (m_userData.Get()->GetId() != response.SENDER_ID)
			{
				UE_LOGFMT(VoxtaLog, Error, "Recieved chat update for an ai character, "
					"let me know if this ever triggers cuz it seemed to be only user-specific. Sender: {0} Content: {1}",
					response.SENDER_ID, response.TEXT_CONTENT);
				return false;
			}

			UE_LOGFMT(VoxtaLog, Log, "Adding user-message to history due to update from VoxtaServer. "
				"MessageId {0} Content: {1}", response.MESSAGE_ID, response.TEXT_CONTENT);

			FChatMessage message = FChatMessage(response.MESSAGE_ID, response.SENDER_ID);
			message.AppendMoreContent(response.TEXT_CONTENT, FString());
			m_chatSession->GetChatMessages().Emplace(MoveTemp(message));

			// we want a pointer after it's moved to the heap, just for safety.
			const FChatMessage* chatMessage = GetChatMessageById(response.MESSAGE_ID);
			VoxtaClientCharMessageAddedEventNative.Broadcast(*m_userData.Get(), *chatMessage);
			VoxtaClientCharMessageAddedEvent.Broadcast(*m_userData.Get(), *chatMessage);
		}
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "Recieved chat update for a different session? This should never happen, I think. "
			"SessionId: {0} SenderId: {1} Content: {2}", response.SESSION_ID, response.SENDER_ID, response.TEXT_CONTENT);
	}
	return true;
}

bool UVoxtaClient::HandleSpeechTranscriptionResponse(const ServerResponseSpeechTranscription& response)
{
	using enum ServerResponseSpeechTranscription::TranscriptionState;
	switch (response.TRANSCRIPTION_STATE)
	{
		case PARTIAL:
			UE_LOGFMT(VoxtaLog, Log, "Received update for partial speech transcription. Current version of "
				"transcription: {0} ", response.TRANSCRIBED_SPEECH);

			VoxtaClientSpeechTranscribedPartialEventNative.Broadcast(response.TRANSCRIBED_SPEECH);
			VoxtaClientSpeechTranscribedPartialEvent.Broadcast(response.TRANSCRIBED_SPEECH);
			break;
		case END:
			if (m_currentState == VoxtaClientState::WaitingForUserReponse)
			{
				UE_LOGFMT(VoxtaLog, Log, "Received finalized version of speech transcription: {0}",
					response.TRANSCRIBED_SPEECH);

				VoxtaClientSpeechTranscribedCompleteEventNative.Broadcast(response.TRANSCRIBED_SPEECH);
				VoxtaClientSpeechTranscribedCompleteEvent.Broadcast(response.TRANSCRIBED_SPEECH);
				SendUserInput(response.TRANSCRIBED_SPEECH);
			}
			// no logging for else, as Transcription sometimes sends the final version more than once, not sure why.
			break;
		case CANCELLED:
			// transcriptions aren't registered unless finalized, so we can just ignore cancellations completely.
			break;
	}
	return true;
}

bool UVoxtaClient::HandleErrorResponse(const ServerResponseError& response)
{
	UE_LOGFMT(VoxtaLog, Error, "Recieved error from the VoxtaServer, please check the logs for additional details. "
		"There might be a misconfiguration on the VoxtaServer side, or you found an edge case (please report it to me). "
		"ErrorMessage: {0}, ErrorDetails: {1}", response.ERROR_MESSAGE, response.ERROR_DETAILS);
	return true;
}

const TUniquePtr<const FAiCharData>* UVoxtaClient::GetAiCharacterDataById(const FString& charId) const
{
	return m_characterList.FindByPredicate([&charId] (const TUniquePtr<const FAiCharData>& inItem)
		{
			return inItem->GetId() == charId;
		});
}

FChatMessage* UVoxtaClient::GetChatMessageById(const FString& messageId) const
{
	return m_chatSession->GetChatMessages().FindByPredicate([&messageId] (const FChatMessage& inItem)
		{
			return inItem.GetMessageId() == messageId;
		});
}

void UVoxtaClient::SetState(VoxtaClientState newState)
{
	UE_LOGFMT(VoxtaLog, Log, "Marking the current VoxtaClient state as: {0}", UEnum::GetValueAsString(newState));

	m_currentState = newState;
	VoxtaClientStateChangedEventNative.Broadcast(m_currentState);
	VoxtaClientStateChangedEvent.Broadcast(m_currentState);
}