// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaClient.h"
#include "SignalR/Public/SignalRSubsystem.h"
#include "VoxtaDefines.h"

UVoxtaClient::UVoxtaClient()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVoxtaClient::BeginPlay()
{
	Super::BeginPlay();
	m_logUtility.RegisterVoxtaLogger();
}

void UVoxtaClient::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (m_currentState != VoxtaClientState::Disconnected)
	{
		Disconnect(true);
	}
}

void UVoxtaClient::StartConnection(const FString& ipv4Address, int port)
{
	if (m_currentState != VoxtaClientState::Disconnected)
	{
		UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient has alread (tried to be) connected, ignoring new connection attempt");
		return;
	}

	m_hostAddress = ipv4Address;
	m_hostPort = port;

	m_hub = GEngine->GetEngineSubsystem<USignalRSubsystem>()->CreateHubConnection(FString::Format(*API_STRING("http://{0}:{1}/hub"), {
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

void UVoxtaClient::StartChatWithCharacter(FString charId)
{
	// TODO: check if character is registered first before trying to initiate a chat
	SendMessageToServer(m_voxtaRequestApi.GetLoadCharacterRequestData(charId));
	SetState(VoxtaClientState::StartingChat);
}

void UVoxtaClient::SendUserInput(FString inputText)
{
	// TODO: check if we are in chatting state before sending user input
	SendMessageToServer(m_voxtaRequestApi.GetSendUserMessageData(m_chatSession->GetSessionId(), inputText));
	SetState(VoxtaClientState::GeneratingReply);
}

void UVoxtaClient::NotifyAudioPlaybackComplete(const FString& messageId)
{
	SendMessageToServer(m_voxtaRequestApi.GetNotifyAudioPlaybackCompleteData(m_chatSession->GetSessionId(), messageId));
	SetState(VoxtaClientState::WaitingForUserReponse);
}

const FChatSession* UVoxtaClient::GetChatSession() const
{
	return m_chatSession.Get();
}

FString UVoxtaClient::GetServerAddress() const
{
	return m_hostAddress;
}

int UVoxtaClient::GetServerPort() const
{
	return m_hostPort;
}

void UVoxtaClient::StartListeningToServer()
{
	m_hub->On(m_receiveMessageEventName).BindUObject(this, &UVoxtaClient::OnReceivedMessage);
	m_hub->OnConnected().AddUObject(this, &UVoxtaClient::OnConnected);
	m_hub->OnConnectionError().AddUObject(this, &UVoxtaClient::OnConnectionError);
	m_hub->OnClosed().AddUObject(this, &UVoxtaClient::OnClosed);
}

void UVoxtaClient::OnReceivedMessage(const TArray<FSignalRValue>& arguments)
{
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([&, Arguments = arguments] (float DeltaTime)
		{
			if (m_currentState == VoxtaClientState::Disconnected || m_currentState == VoxtaClientState::Terminated)
			{
				UE_LOGFMT(VoxtaLog, Warning, "Tried to process a message with the connection already severed, skipping processing of remaining response data.");
				return false;
			}
			if (Arguments.IsEmpty() || Arguments[0].GetType() != FSignalRValue::EValueType::Object)
			{
				UE_LOGFMT(VoxtaLog, Error, "Received invalid message from server.");
			}
			else if (!HandleResponse(Arguments[0].AsObject()))
			{
				UE_LOGFMT(VoxtaLog, Warning, "Received server response that is not (yet) supported: {type}",
					Arguments[0].AsObject()[API_STRING("$type")].AsString());
			}
			return false;
		}));
}

void UVoxtaClient::OnConnected()
{
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([&] (float DeltaTime)
		{
			UE_LOGFMT(VoxtaLog, Log, "VoxtaClient connected successfully");
			SendMessageToServer(m_voxtaRequestApi.GetAuthenticateRequestData());
			return false;
		}));
}

void UVoxtaClient::OnConnectionError(const FString& error)
{
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([&, Error = error] (float DeltaTime)
		{
			UE_LOGFMT(VoxtaLog, Error, "VoxtaClient connection has encountered error: {error}.", Error);
			Disconnect();
			return false;
		}));
}

void UVoxtaClient::OnClosed()
{
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([&] (float DeltaTime)
		{
			UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient connection has been closed.");
			Disconnect();
			return false;
		}));
}

void UVoxtaClient::SendMessageToServer(const FSignalRValue& message)
{
	m_hub->Invoke(m_sendMessageEventName, message).BindUObject(this, &UVoxtaClient::OnMessageSent);
}

void UVoxtaClient::OnMessageSent(const FSignalRInvokeResult& result)
{
	if (result.HasError())
	{
		UE_LOGFMT(VoxtaLog, Error, "Failed to send message due to error: {err}.", result.GetErrorMessage());
	}
}

template<typename T>
bool UVoxtaClient::HandleResponseHelper(const IServerResponseBase* response, const char* message, bool (UVoxtaClient::* handler)(const T&))
{
	auto derivedResponse = StaticCast<const T*>(response);
	if (derivedResponse)
	{
		UE_LOG(VoxtaLog, Log, TEXT("%s"), *FString(message));
		return (this->*handler)(*derivedResponse);
	}
	return false;
}

bool UVoxtaClient::HandleResponse(const TMap<FString, FSignalRValue>& responseData)
{
	if (m_voxtaResponseApi.ignoredMessageTypes.Contains(responseData[API_STRING("$type")].AsString()))
	{
		return true;
	}

	auto response = m_voxtaResponseApi.GetResponseData(responseData);
	if (!response)
	{
		return false;
	}

	switch (response->GetType())
	{
		using enum ServerResponseType;
		case Welcome:
			return HandleResponseHelper<ServerResponseWelcome>(response.Get(), "Logged in successfully", &UVoxtaClient::HandleWelcomeResponse);
		case CharacterList:
			return HandleResponseHelper<ServerResponseCharacterList>(response.Get(), "Fetched characters successfully", &UVoxtaClient::HandleCharacterListResponse);
		case CharacterLoaded:
			return HandleResponseHelper<ServerResponseCharacterLoaded>(response.Get(), "Loaded character successfully", &UVoxtaClient::HandleCharacterLoadedResponse);
		case ChatStarted:
			return HandleResponseHelper<ServerResponseChatStarted>(response.Get(), "Chat started successfully", &UVoxtaClient::HandleChatStartedResponse);
		case ChatMessage:
			return HandleResponseHelper<IServerResponseChatMessageBase>(response.Get(), "Chat Message received successfully", &UVoxtaClient::HandleChatMessageResponse);
		case ChatUpdate:
			return HandleResponseHelper<ServerResponseChatUpdate>(response.Get(), "Chat Update received successfully", &UVoxtaClient::HandleChatUpdateResponse);
		case SpeechTranscription:
			return HandleResponseHelper<ServerResponseSpeechTranscription>(response.Get(), "Speech transcription update received successfully", &UVoxtaClient::HandleSpeechTranscriptionResponse);
		default:
			return false;
	}
	return false;
}

bool UVoxtaClient::HandleWelcomeResponse(const ServerResponseWelcome& response)
{
	m_userData = MakeUnique<FUserCharData>(response.m_user);
	UE_LOGFMT(VoxtaLog, Log, "Authenticated with Voxta Server. Welcome {user}.", m_userData->GetName());
	SetState(VoxtaClientState::Authenticated);
	SendMessageToServer(m_voxtaRequestApi.GetLoadCharactersListData());
	return true;
}

bool UVoxtaClient::HandleCharacterListResponse(const ServerResponseCharacterList& response)
{
	m_characterList.Empty();
	for (auto& charElement : response.m_characters)
	{
		m_characterList.Emplace(MakeUnique<FAiCharData>(charElement));
		VoxtaClientCharacterRegisteredEvent.Broadcast(charElement);
	}
	SetState(VoxtaClientState::Idle);
	return true;
}

bool UVoxtaClient::HandleCharacterLoadedResponse(const ServerResponseCharacterLoaded& response)
{
	auto character = m_characterList.FindByPredicate([response] (const TUniquePtr<const FAiCharData>& InItem)
		{
			return InItem->GetId() == response.m_characterId;
		});

	if (character)
	{
		SendMessageToServer(m_voxtaRequestApi.GetStartChatRequestData(character->Get()));
		return true;
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Loaded a character ({char}) that doesn't exist in the list? This should never happen..", response.m_characterId);
		return false;
	}
}

bool UVoxtaClient::HandleChatStartedResponse(const ServerResponseChatStarted& response)
{
	TArray<const FAiCharData*> characters;
	for (const TUniquePtr<const FAiCharData>& UniquePtr : m_characterList)
	{
		if (response.m_characterIds.Contains(UniquePtr->GetId()))
		{
			characters.Add(UniquePtr.Get());
		}
	}

	if (!response.m_services.Contains(VoxtaServiceData::ServiceType::SpeechToText))
	{
		UE_LOGFMT(VoxtaLog, Error, "No valid SpeechToText service is active on the server.");
		return false;
	}
	if (!response.m_services.Contains(VoxtaServiceData::ServiceType::TextGen))
	{
		UE_LOGFMT(VoxtaLog, Error, "No valid TextGen service is active on the server.");
		return false;
	}
	if (!response.m_services.Contains(VoxtaServiceData::ServiceType::TextToSpeech))
	{
		UE_LOGFMT(VoxtaLog, Error, "No valid TextToSpeech service is active on the server.");
		return false;
	}

	m_chatSession = MakeUnique<FChatSession>(characters, response.m_chatId,
		response.m_sessionId, response.m_services);
	VoxtaClientChatSessionStartedEvent.Broadcast(*m_chatSession.Get());
	SetState(VoxtaClientState::GeneratingReply);
	return true;
}

bool UVoxtaClient::HandleChatMessageResponse(const IServerResponseChatMessageBase& response)
{
	auto& messages = m_chatSession->m_chatMessages;
	using enum IServerResponseChatMessageBase::MessageType;
	switch (response.GetMessageType())
	{
		case MessageStart:
		{
			auto derivedResponse = StaticCast<const ServerResponseChatMessageStart*>(&response);
			messages.Emplace(FChatMessage(
				derivedResponse->m_messageId, derivedResponse->m_senderId));
			break;
		}
		case MessageChunk:
		{
			auto derivedResponse = StaticCast<const ServerResponseChatMessageChunk*>(&response);
			auto chatMessage = messages.FindByPredicate([derivedResponse] (const FChatMessage& InItem)
				{
					return InItem.GetMessageId() == derivedResponse->m_messageId;
				});
			if (chatMessage)
			{
				(*chatMessage).m_text.Append((*chatMessage).m_text.IsEmpty() ? derivedResponse->m_messageText
					: FString::Format(*API_STRING(" {0}"), { derivedResponse->m_messageText }));
				(*chatMessage).m_audioUrls.Emplace(derivedResponse->m_audioUrlPath);
			}
			break;
		}
		case MessageEnd:
		{
			auto derivedResponse = StaticCast<const ServerResponseChatMessageEnd*>(&response);
			auto chatMessage = messages.FindByPredicate([derivedResponse] (const FChatMessage& InItem)
				{
					return InItem.GetMessageId() == derivedResponse->m_messageId;
				});
			if (chatMessage)
			{
				auto character = m_characterList.FindByPredicate([derivedResponse] (const TUniquePtr<const FAiCharData>& InItem)
					{
						return InItem->GetId() == derivedResponse->m_senderId;
					});
				if (character)
				{
					UE_LOGFMT(VoxtaLog, Log, "Char speaking message end: {0} -> {1}", character->Get()->GetName(), chatMessage->m_text);
					SetState(VoxtaClientState::AudioPlayback);
					VoxtaClientCharMessageAddedEvent.Broadcast(*character->Get(), *chatMessage);
				}
			}
			break;
		}
		case MessageCancelled:
		{
			auto derivedResponse = StaticCast<const ServerResponseChatMessageCancelled*>(&response);
			int index = messages.IndexOfByPredicate([derivedResponse] (const FChatMessage& InItem)
				{
					return InItem.GetMessageId() == derivedResponse->m_messageId;
				});
			VoxtaClientCharMessageRemovedEvent.Broadcast(messages[index]);

			messages.RemoveAt(index);
		}
	}
	return true;
}

bool UVoxtaClient::HandleChatUpdateResponse(const ServerResponseChatUpdate& response)
{
	if (response.m_sessionId == m_chatSession->GetSessionId())
	{
		if (m_chatSession->m_chatMessages.ContainsByPredicate([response] (const FChatMessage& InItem)
			{
				return InItem.GetMessageId() == response.m_messageId;
			}))
		{
			UE_LOGFMT(VoxtaLog, Error, "Recieved chat update for a message that already exists, needs implementation. {0} {1}", response.m_senderId, response.m_text);
		}
		else
		{
			m_chatSession->m_chatMessages.Emplace(FChatMessage(response.m_messageId, response.m_senderId, response.m_text));
			if (m_userData.Get()->GetId() != response.m_senderId)
			{
				UE_LOGFMT(VoxtaLog, Error, "Recieved chat update for a non-user character, needs implementation. {0} {1}", response.m_senderId, response.m_text);
			}
			auto chatMessage = m_chatSession->m_chatMessages.FindByPredicate([response] (const FChatMessage& InItem)
				{
					return InItem.GetMessageId() == response.m_messageId;
				});
			UE_LOGFMT(VoxtaLog, Log, "Char speaking message end: {0} -> {1}", m_userData.Get()->GetName(), chatMessage->m_text);
			VoxtaClientCharMessageAddedEvent.Broadcast(*m_userData.Get(), *chatMessage);
		}
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "Recieved chat update for a different session?: {0} {1}", response.m_senderId, response.m_text);
	}
	return true;
}

bool UVoxtaClient::HandleSpeechTranscriptionResponse(const ServerResponseSpeechTranscription& response)
{
	using enum ServerResponseSpeechTranscription::TranscriptionState;
	switch (response.m_transcriptionState)
	{
		case PARTIAL:
			UE_LOGFMT(VoxtaLog, Log, "Received partial speech transcription update: {0} ", response.m_transcribedSpeech);
			VoxtaClientSpeechTranscribedPartialEvent.Broadcast(response.m_transcribedSpeech);
			break;
		case END:
			if (m_currentState == VoxtaClientState::WaitingForUserReponse)
			{
				VoxtaClientSpeechTranscribedCompleteEvent.Broadcast(response.m_transcribedSpeech);
				SendUserInput(response.m_transcribedSpeech);
			}
			break;
		case CANCELLED:
			// Just ignore cancelled statuses right now,
			// for some reason server says it's cancelled but then it picks back up again, idk why.
			// TODO
			break;
	}
	return true;
}

void UVoxtaClient::SetState(VoxtaClientState newState)
{
	m_currentState = newState;
	VoxtaClientStateChangedEvent.Broadcast(m_currentState);
}