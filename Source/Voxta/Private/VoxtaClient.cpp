// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaClient.h"
#include "SignalRSubsystem.h"
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
		Disconnect();
	}
}

void UVoxtaClient::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UVoxtaClient::StartConnection()
{
	if (m_currentState != VoxtaClientState::Disconnected)
	{
		UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient has alread (tried to be) connected, ignoring new connection attempt");
		return;
	}

	SetState(VoxtaClientState::Connecting);
	m_hub = GEngine->GetEngineSubsystem<USignalRSubsystem>()->CreateHubConnection(FString::Format(*API_STRING("http://{0}:{1}/hub"), {
		TEXT("192.168.178.8"),
		TEXT("5384")
		}));

	StartListeningToServer();
	m_hub->Start();

	UE_LOGFMT(VoxtaLog, Log, "Starting Voxta client");
}

void UVoxtaClient::Disconnect()
{
	if (m_currentState == VoxtaClientState::Disconnected)
	{
		UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient is currently not connected, ignoring disconnect attempt");
		return;
	}
	SetState(VoxtaClientState::Terminated);
	m_hub->Stop();
}

void UVoxtaClient::LoadCharacter(FString charID)
{
	SendMessageToServer(m_voxtaRequestApi.GetLoadCharacterRequestData(charID));
	SetState(VoxtaClientState::StartingChat);
}

void UVoxtaClient::SendUserInput(FString inputText)
{
	SendMessageToServer(m_voxtaRequestApi.GetSendUserMessageData(m_chatSession->m_sessionId, inputText));
}

void UVoxtaClient::StartListeningToServer()
{
	m_hub->On(m_receiveMessageEventName).BindUObject(this, &UVoxtaClient::OnReceivedMessage);
	m_hub->OnConnected().AddUObject(this, &UVoxtaClient::OnConnected);
	m_hub->OnConnectionError().AddUObject(this, &UVoxtaClient::OnConnectionError);
	m_hub->OnClosed().AddUObject(this, &UVoxtaClient::OnClosed);
}

void UVoxtaClient::OnReceivedMessage(const TArray<FSignalRValue>& Arguments)
{
	if (m_currentState == VoxtaClientState::Disconnected || m_currentState == VoxtaClientState::Terminated)
	{
		UE_LOGFMT(VoxtaLog, Warning, "Tried to process a message with the connection already severed, skipping processing of remaining response data.");
		return;
	}
	if (Arguments.IsEmpty() || Arguments[0].GetType() != FSignalRValue::EValueType::Object)
	{
		UE_LOGFMT(VoxtaLog, Error, "Received invalid message from server.");
	}
	else try
	{
		if (!HandleResponse(Arguments[0].AsObject()))
		{
			UE_LOGFMT(VoxtaLog, Warning, "Received server response that is not (yet) supported: {type}",
				Arguments[0].AsObject()[API_STRING("$type")].AsString());
		}
	}
	catch (const std::exception& ex)
	{
		FString error(TEXT("Something went wrong while parsing the server response: "));
		error += ex.what();
		UE_LOGFMT(VoxtaLog, Error, "Received server response that is not (yet) supported: {error}", error);
	}
}

void UVoxtaClient::OnConnected()
{
	UE_LOGFMT(VoxtaLog, Log, "VoxtaClient connected successfully");
	SendMessageToServer(m_voxtaRequestApi.GetAuthenticateRequestData());
}

void UVoxtaClient::OnConnectionError(const FString& error)
{
	UE_LOGFMT(VoxtaLog, Error, "VoxtaClient connection has encountered error: {error}.", error);
	Disconnect();
}

void UVoxtaClient::OnClosed()
{
	UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient connection has been closed.");
	Disconnect();
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
		{
			auto derivedResponse = StaticCast<const ServerResponseWelcome*>(response.Get());
			if (derivedResponse)
			{
				UE_LOGFMT(VoxtaLog, Log, "Logged in sucessfully");
				HandleWelcomeResponse(*derivedResponse);
				return true;
			}
		}
		case CharacterList:
		{
			auto derivedResponse = StaticCast<const ServerResponseCharacterList*>(response.Get());
			if (derivedResponse)
			{
				UE_LOGFMT(VoxtaLog, Log, "Fetched {count} characters sucessfully", derivedResponse->m_characters.Num());
				HandleCharacterListResponse(*derivedResponse);
				return true;
			}
		}
		case CharacterLoaded:
		{
			auto derivedResponse = StaticCast<const ServerResponseCharacterLoaded*>(response.Get());
			if (derivedResponse)
			{
				UE_LOGFMT(VoxtaLog, Log, "Loaded character {char} sucessfully", derivedResponse->m_characterId);
				HandleCharacterLoadedResponse(*derivedResponse);
				return true;
			}
		}
		case ChatStarted:
		{
			auto derivedResponse = StaticCast<const ServerResponseChatStarted*>(response.Get());
			if (derivedResponse)
			{
				UE_LOGFMT(VoxtaLog, Log, "Chat started sucessfully");
				HandleChatStartedResponse(*derivedResponse);
				return true;
			}
		}
		case ChatMessage:
		{
			auto derivedResponse = StaticCast<const ServerResponseChatMessage*>(response.Get());
			if (derivedResponse)
			{
				UE_LOGFMT(VoxtaLog, Log, "Chat Message received sucessfully");
				HandleChatMessageResponse(*derivedResponse);
				return true;
			}
		}
		case ChatUpdate:
		{
			auto derivedResponse = StaticCast<const ServerResponseChatUpdate*>(response.Get());
			if (derivedResponse)
			{
				UE_LOGFMT(VoxtaLog, Log, "Chat Update received sucessfully");
				HandleChatUpdateResponse(*derivedResponse);
				return true;
			}
		}
		case SpeechTranscription:
		default:
			return false;
	}
	return false;
}

void UVoxtaClient::HandleWelcomeResponse(const ServerResponseWelcome& response)
{
	m_userData = MakeUnique<FCharData>(response.m_user);
	UE_LOGFMT(VoxtaLog, Log, "Authenticated with Voxta Server. Welcome {user}.", m_userData->m_name);
	SetState(VoxtaClientState::Authenticated);
	SendMessageToServer(m_voxtaRequestApi.GetLoadCharactersListData());
}

void UVoxtaClient::HandleCharacterListResponse(const ServerResponseCharacterList& response)
{
	m_characterList.Empty();
	for (auto& charElement : response.m_characters)
	{
		m_characterList.Emplace(MakeUnique<FCharData>(charElement));
		OnVoxtaClientCharacterLoadedDelegate.Broadcast(charElement);
	}
	SetState(VoxtaClientState::CharacterLobby);
}

bool UVoxtaClient::HandleCharacterLoadedResponse(const ServerResponseCharacterLoaded& response)
{
	auto character = m_characterList.FindByPredicate([response] (const TUniquePtr<const FCharData>& InItem)
	{
		return InItem->m_id == response.m_characterId;
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
	TArray<const FCharData*> characters;
	for (const TUniquePtr<const FCharData>& UniquePtr : m_characterList)
	{
		if (response.m_characterIds.Contains(UniquePtr->m_id))
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

	m_chatSession = MakeUnique<ChatSession>(characters, response.m_chatId,
		response.m_sessionId, response.m_services);
	SetState(VoxtaClientState::Chatting);
	return true;
}

void UVoxtaClient::HandleChatMessageResponse(const ServerResponseChatMessage& response)
{
	auto& messages = m_chatSession->m_chatMessages;
	using enum ServerResponseChatMessage::MessageType;
	switch (response.m_messageType)
	{
		case MESSAGE_START:
		{
			messages.Emplace(MakeUnique<FChatMessage>(
				response.m_messageId, response.m_senderId));
			break;
		}
		case MESSAGE_CHUNK:
		{
			auto chatMessage = messages.FindByPredicate([response] (const TUniquePtr<FChatMessage>& InItem)
			{
				return InItem->m_messageId == response.m_messageId;
			});
			if (chatMessage)
			{
				(*chatMessage)->m_text.Append((*chatMessage)->m_text.IsEmpty() ? response.m_messageText
					: FString::Format(*API_STRING(" {0}"), { response.m_messageText }));
				(*chatMessage)->m_audioUrls.Emplace(response.m_audioUrlPath);
			}
			break;
		}
		case MESSAGE_END:
		{
			auto chatMessage = messages.FindByPredicate([response] (const TUniquePtr<FChatMessage>& InItem)
			{
				return InItem->m_messageId == response.m_messageId;
			});
			if (chatMessage)
			{
				auto character = m_characterList.FindByPredicate([response] (const TUniquePtr<const FCharData>& InItem)
				{
					return InItem->m_id == response.m_senderId;
				});
				if (character)
				{
					UE_LOGFMT(VoxtaLog, Log, "Char speaking message end: {0} -> {1}", character->Get()->m_name, chatMessage->Get()->m_text);
					OnVoxtaClientChatMessageAdded.Broadcast(*character->Get(), *chatMessage->Get());
				}
			}
			break;
		}
		case MESSAGE_CANCELLED:
		{
			int index = messages.IndexOfByPredicate([response] (const TUniquePtr<FChatMessage>& InItem)
			{
				return InItem->m_messageId == response.m_messageId;
			});
			OnVoxtaClientChatMessageRemoved.Broadcast(*messages[index].Get());

			messages[index].Reset();
			messages.RemoveAt(index);
		}
	}
}

void UVoxtaClient::HandleChatUpdateResponse(const ServerResponseChatUpdate& response)
{
	if (response.m_sessionId == m_chatSession->m_sessionId)
	{
		if (m_chatSession->m_chatMessages.ContainsByPredicate([response] (const TUniquePtr<FChatMessage>& InItem)
			{
				return InItem->m_messageId == response.m_messageId;
			}))
		{
			UE_LOGFMT(VoxtaLog, Error, "Recieved chat update for a message that already exists, needs implementation. {0} {1}", response.m_senderId, response.m_text);
		}
		else
		{
			m_chatSession->m_chatMessages.Emplace(MakeUnique<FChatMessage>(response.m_messageId, response.m_senderId, response.m_text));
			if (m_userData.Get()->m_id != response.m_senderId)
			{
				UE_LOGFMT(VoxtaLog, Error, "Recieved chat update for a non-user character, needs implementation. {0} {1}", response.m_senderId, response.m_text);
			}
			auto chatMessage = m_chatSession->m_chatMessages.FindByPredicate([response] (const TUniquePtr<FChatMessage>& InItem)
			{
				return InItem->m_messageId == response.m_messageId;
			});
			UE_LOGFMT(VoxtaLog, Log, "Char speaking message end: {0} -> {1}", m_userData.Get()->m_name, chatMessage->Get()->m_text);
			OnVoxtaClientChatMessageAdded.Broadcast(*m_userData.Get(), *chatMessage->Get());
		}
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "Recieved chat update for a different session?: {0} {1}", response.m_senderId, response.m_text);
	}
}

void UVoxtaClient::SetState(VoxtaClientState newState)
{
	m_currentState = newState;
	OnVoxtaClientStateChangedDelegate.Broadcast(m_currentState);
}