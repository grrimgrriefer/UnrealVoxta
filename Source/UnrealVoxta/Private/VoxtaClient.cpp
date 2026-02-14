// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaClient.h"
#include "SignalR/Public/SignalRSubsystem.h"
#include "SignalR/Private/HubConnection.h"
#include "Audio2FaceRESTHandler.h"
#include "VoxtaDefines.h"
#include "Logging/StructuredLog.h"
#include "VoxtaAudioInput.h"
#include "VoxtaAudioPlayback.h"
#include "VoxtaGlobalAudioPlaybackHolder.h"
#include "VoxtaGlobalAudioPlayback.h"
#include "VoxtaLogger.h"
#include "VoxtaApiRequestHandler.h"
#include "VoxtaApiResponseHandler.h"
#include "TexturesCacheHandler.h"
#include "VoxtaHelperFunctionLibrary.h"
#include "VoxtaData/Public/ChatSession.h"
#include "VoxtaData/Public/VoxtaVersionData.h"
#include "VoxtaData/Public/ServerResponses.h"
#include "LogUtility/Public/Defines.h"

void UVoxtaClient::Initialize(FSubsystemCollectionBase& collection)
{
	m_logUtility = MakeShared<VoxtaLogger>();
	m_logUtility->RegisterVoxtaLogger();
	m_voiceInput = NewObject<UVoxtaAudioInput>(this);
	m_A2FHandler = MakeShared<Audio2FaceRESTHandler>();
	m_texturesCacheHandler = MakeShared<TexturesCacheHandler>();
	m_globalAudioPlaybackComp = nullptr;
	SensitiveLogging::isSensitiveLogsCensored = true;
	Super::Initialize(collection);
}

void UVoxtaClient::Deinitialize()
{
	if (m_globalAudioPlaybackComp != nullptr)
	{
		if (m_globalAudioPlaybackComp->GetGlobalPlaybackComponent() != nullptr && globalAudioPlaybackHandle.IsValid())
		{
			m_globalAudioPlaybackComp->GetGlobalPlaybackComponent()->VoxtaMessageAudioPlaybackFinishedEventNative.Remove(globalAudioPlaybackHandle);
		}
		if (UWorld* World = GetWorld())
		{
			World->RemoveActor(m_globalAudioPlaybackComp, true);
		}
	}

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

	if (!UVoxtaHelperFunctionLibrary::IsIpv4Valid(ipv4Address))
	{
		SENSITIVE_LOG1(VoxtaLog, Error, "Address: {0} is not a valid address. Ignoring connection attempt.", ipv4Address);
		return;
	}

	m_hostAddress = (ipv4Address.ToLower() == UVoxtaHelperFunctionLibrary::LOCALHOST) ? TEXT("127.0.0.1") : ipv4Address;
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
	if (m_voiceInput != nullptr && m_voiceInput->IsInitialized())
	{
		m_voiceInput->Cleanup();
	}

	if (m_currentState == VoxtaClientState::Disconnected)
	{
		UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient is currently not connected, ignoring disconnect attempt");
		return;
	}
	if (silent)
	{
		m_currentState = VoxtaClientState::Terminated;
	}
	else
	{
		SetState(VoxtaClientState::Terminated);
	}
	StopChatInternal();
	if (m_hub.IsValid())
	{
		m_hub->Stop();
	}
}

void UVoxtaClient::StartChatWithCharacter(const FGuid& charId, const FString& context)
{
	if (!charId.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Error, "Cannot start a chat as the provided characterId was empty.");
		return;
	}
	if (m_currentState != VoxtaClientState::Idle)
	{
		UE_LOGFMT(VoxtaLog, Error, "Cannot start a chat as the current state: {0}, is not Idle. (requested character {1})",
			UEnum::GetValueAsString(m_currentState), GuidToString(charId));
		return;
	}
	const TUniquePtr<const FAiCharData>* character = GetAiCharacterDataById(charId);
	if (character != nullptr && character->IsValid())
	{
		SendMessageToServer(VoxtaApiRequestHandler::GetStartChatRequestData(character->Get(), context));
		GetOrCreateGlobalAudioFallbackInternal();

		SetState(VoxtaClientState::StartingChat);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Cannot start a chat with characterId {0} as it's not in the current character list.",
			GuidToString(charId));
	}
}

void UVoxtaClient::SetGlobalAudioFallbackEnabled(bool newState)
{
	if (AVoxtaGlobalAudioPlaybackHolder* holder = GetOrCreateGlobalAudioFallbackInternal())
	{
		if (UVoxtaGlobalAudioPlayback* comp = holder->GetGlobalPlaybackComponent())
		{
			comp->SetEnabled(newState);
		}
		else
		{
			UE_LOGFMT(VoxtaLog, Warning, "Failed to enable/disable global audio fallback, playback component is unavailable.");
		}
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "Failed to enable/disable global audio fallback, global playback holder is unavailable.");
	}
}

void UVoxtaClient::StopActiveChat()
{
	SendMessageToServer(VoxtaApiRequestHandler::GetStopChatRequestData());
}

void UVoxtaClient::UpdateChatContext(const FString& newContext)
{
	if (m_chatSession.IsValid())
	{
		SendMessageToServer(VoxtaApiRequestHandler::GetUpdateContextRequestData(m_chatSession->GetSessionId(), newContext));
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "Cannot update context of chatSession, chat was not valid? Current state: {0}",
			UEnum::GetValueAsString(m_currentState));
	}
}

void UVoxtaClient::SendUserInput(const FString& inputText, bool generateReply, bool characterActionInference)
{
	if (m_currentState == VoxtaClientState::WaitingForUserResponse)
	{
		if (!m_chatSession.IsValid())
		{
			UE_LOGFMT(VoxtaLog, Error, "Cannot send userinput, chat was not valid? Current state: {1}", UEnum::GetValueAsString(m_currentState));
			return;
		}

		SendMessageToServer(VoxtaApiRequestHandler::GetSendUserMessageData(m_chatSession->GetSessionId(),
			inputText, generateReply, characterActionInference));
		SetState(VoxtaClientState::GeneratingReply);
	}
	else
	{
		SENSITIVE_LOG2(VoxtaLog, Error, "Cannot send userInput {0} as the VoxtaServer is currently {1}, "
			"please wait untile it's WaitingForUserResponse.", inputText, UEnum::GetValueAsString(m_currentState));
	}
}

void UVoxtaClient::NotifyAudioPlaybackComplete(const FGuid& messageId)
{
	if (!m_chatSession.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Warning, "Cannot notify AudioPlayback completion as there's no active chat session.");
		return;
	}

	if (m_currentState == VoxtaClientState::AudioPlayback)
	{
		UE_LOGFMT(VoxtaLog, Log, "Marking audio playback of message {0} complete.", GuidToString(messageId));
	}
	else if (m_currentState == VoxtaClientState::GeneratingReply && !IsGlobalAudioFallbackActive())
	{
		UE_LOGFMT(VoxtaLog, Log, "Skipping audio playback of message {0} due to configuration.", GuidToString(messageId));
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Tried to mark AudioPlayback as complete, but we weren't in the audioPlayback state,"
			" actual state: {0}, messageId tried to mark as complete: {1}", UEnum::GetValueAsString(m_currentState), GuidToString(messageId));
		return;
	}

	SendMessageToServer(VoxtaApiRequestHandler::GetNotifyAudioPlaybackCompletedData(m_chatSession->GetSessionId(), messageId));
	SetState(VoxtaClientState::WaitingForUserResponse);
}

void UVoxtaClient::TryFetchAndCacheCharacterThumbnail(const FGuid& baseCharacterId, FDownloadedTextureDelegateNative onThumbnailFetched)
{
	if (!baseCharacterId.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Error, "Cannot fetch the thumbnail for character due to invalid id: {0}.",
			GuidToString(baseCharacterId));
		return;
	}
	const FBaseCharData* character = nullptr;
	if (!m_userData.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Error, "Cannot fetch thumbnail, user metadata not yet available.");
		return;
	}

	if (baseCharacterId == m_userData->GetId())
	{
		character = m_userData.Get();
	}
	else
	{
		const TUniquePtr<const FAiCharData>* aiCharacter = GetAiCharacterDataById(baseCharacterId);
		if (aiCharacter == nullptr)
		{
			UE_LOGFMT(VoxtaLog, Error, "Cannot fetch the thumbnail for character with id: {0}, as it isn't present in the "
				"current character list and it isn't the current user", GuidToString(baseCharacterId));
			return;
		}
		character = aiCharacter->Get();
	}
	const FStringView charThumnailUrl = character->GetThumbnailUrl();
	if (!charThumnailUrl.IsEmpty())
	{
		FString url = FString::Format(TEXT("http://{0}:{1}{2}"),
			{ m_hostAddress, m_hostPort, charThumnailUrl });
		SENSITIVE_LOG1(VoxtaLog, Log, "Loading thumbnail from URL: {0}", url);

		m_texturesCacheHandler->FetchTextureFromUrl(url, onThumbnailFetched);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Log, "Character has no custom thumbnail: {0}", GuidToString(baseCharacterId));
		if (onThumbnailFetched.IsBound())
		{
			onThumbnailFetched.Execute(false, nullptr, FIntVector2());
		}
	}
}

bool UVoxtaClient::TryRegisterPlaybackHandler(const FGuid& characterId,
	TWeakObjectPtr<UVoxtaAudioPlayback> playbackHandler)
{
	if (!playbackHandler.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Error, "You tried to register a Voxta AudioPlayback handler that was null, for the "
			"character with id {0}.", GuidToString(characterId));
		return false;
	}

	if (m_chatSession.IsValid() && !m_chatSession->GetActiveServices().Contains(VoxtaServiceType::TextToSpeech))
	{
		UE_LOGFMT(VoxtaLog, Warning, "You tried to register a Voxta AudioPlayback handler for character {0}, but no "
			"TTS service is active on VoxtaServer. (make sure to have it enabled at start; runtime activation not yet "
			"supported...)", GuidToString(characterId));
		return false;
	}

	auto currentHandler = m_registeredCharacterAudioPlaybackComps.Find(characterId);
	if (currentHandler == nullptr)
	{
		m_registeredCharacterAudioPlaybackComps.Emplace(characterId, playbackHandler);
		UE_LOGFMT(VoxtaLog, Log, "Voxta Audioplayback handler for character: {0} registered successfully.", GuidToString(characterId));

		if (playbackHandler->GetLipSyncType() == LipSyncType::Audio2Face)
		{
			UE_LOGFMT(VoxtaLog, Log, "Voxta Audioplayback handler of character: {0} requires A2F, trying to establish "
				"connection... hold on.", GuidToString(characterId));
			m_A2FHandler->TryInitialize();
		}

		m_audioPlaybackHandles.Emplace(characterId, playbackHandler->VoxtaMessageAudioPlaybackFinishedEventNative.AddUObject(this, &UVoxtaClient::NotifyAudioPlaybackComplete));
		VoxtaClientAudioPlaybackRegisteredEventNative.Broadcast(playbackHandler.Get(), characterId);
		VoxtaClientAudioPlaybackRegisteredEvent.Broadcast(playbackHandler.Get(), characterId);
		return true;
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "A Voxta Audioplayback handler for character: {0} already exists. "
			"Multiple audio playback handlers for the same character is not supported... skipping registration.",
			GuidToString(characterId));
		return false;
	}
}

bool UVoxtaClient::TryUnregisterPlaybackHandler(const FGuid& characterId)
{
	TWeakObjectPtr<UVoxtaAudioPlayback>* audioPlaybackComp = m_registeredCharacterAudioPlaybackComps.Find(characterId);
	FDelegateHandle* handle = m_audioPlaybackHandles.Find(characterId);
	if (audioPlaybackComp != nullptr && audioPlaybackComp->IsValid())
	{
		if (handle != nullptr)
		{
			audioPlaybackComp->Get()->VoxtaMessageAudioPlaybackFinishedEventNative.Remove(*handle);
		}
		m_registeredCharacterAudioPlaybackComps.Remove(characterId);
		m_audioPlaybackHandles.Remove(characterId);
		UE_LOGFMT(VoxtaLog, Log, "Voxta Audioplayback handler for character: {0} unregistered successfully.", GuidToString(characterId));
		return true;
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "Tried to remove Audioplayback handler for character: {0}, but it was not valid or not registered",
			GuidToString(characterId));
		return false;
	}
}

const FString& UVoxtaClient::GetServerAddress() const
{
	return m_hostAddress;
}

int UVoxtaClient::GetServerPort() const
{
	return m_hostAddress != FString() ? m_hostPort : -1;
}

UVoxtaAudioInput* UVoxtaClient::GetVoiceInputHandler() const
{
	return m_voiceInput;
}

VoxtaClientState UVoxtaClient::GetCurrentState() const
{
	return m_currentState;
}

FGuid UVoxtaClient::GetUserId() const
{
	if (m_userData.IsValid())
	{
		return m_userData->GetId();
	}
	return FGuid();
}

FString UVoxtaClient::GetBrowserUrlForCharacter(const FGuid& aiCharacterId) const
{
	if (GetAiCharacterDataById(aiCharacterId) != nullptr)
	{
		return FString::Format(TEXT("http://{0}:{1}/characters/{2}"),
			{ m_hostAddress, m_hostPort, GuidToString(aiCharacterId) });
	}
	return FString();
}

FGuid UVoxtaClient::GetMainAssistantId() const
{
	return m_mainAssistantId;
}

const UVoxtaAudioPlayback* UVoxtaClient::GetRegisteredAudioPlaybackHandlerForID(const FGuid& characterId) const
{
	auto currentHandler = m_registeredCharacterAudioPlaybackComps.Find(characterId);
	if (currentHandler != nullptr)
	{
		return currentHandler->Get();
	}
	else
	{
		return nullptr;
	}
}

FChatSession UVoxtaClient::GetChatSessionCopy() const
{
	const FChatSession* session = GetChatSession();
	if (session == nullptr)
	{
		return FChatSession();
	}
	return *session;
}

FVoxtaVersionData UVoxtaClient::GetServerVersionCopy() const
{
	if (!m_voxtaVersionData.IsValid())
	{
		return FVoxtaVersionData();
	}
	return *m_voxtaVersionData;
}

bool UVoxtaClient::IsMatchingAPIVersion() const
{
	if (m_voxtaVersionData.IsValid())
	{
		return m_voxtaVersionData->IsMatchingAPIVersion();
	}
	return false;
}

const FChatSession* UVoxtaClient::GetChatSession() const
{
	if (m_chatSession.IsValid())
	{
		return m_chatSession.Get();
	}
	return nullptr;
}

TWeakPtr<Audio2FaceRESTHandler> UVoxtaClient::GetA2FHandler() const
{
	return m_A2FHandler;
}

void UVoxtaClient::SetCensoredLogs(bool isCensorActive)
{
	SensitiveLogging::isSensitiveLogsCensored = isCensorActive;
}

bool UVoxtaClient::IsLogCensorActive() const
{
	return SensitiveLogging::isSensitiveLogsCensored;
}

bool UVoxtaClient::IsGlobalAudioFallbackActive() const
{
	if (m_globalAudioPlaybackComp == nullptr)
	{
		return false;
	}
	UVoxtaGlobalAudioPlayback* comp = m_globalAudioPlaybackComp->GetGlobalPlaybackComponent();
	if (comp != nullptr)
	{
		return comp->IsEnabled();
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "VoxtaGlobalAudioPlayback was null on the holder, this should never happen. Likely GC issue.");
		return false;
	}
}

FAiCharData UVoxtaClient::GetAiCharacterDataCopyById(const FGuid& characterId) const
{
	const TUniquePtr<const FAiCharData>* character = GetAiCharacterDataById(characterId);
	if (character == nullptr || !character->IsValid())
	{
		UE_LOGFMT(VoxtaLog, Error, "GetAiCharacterDataCopyById could not find a character that matched id {0}", characterId);
		return FAiCharData();
	}
	else
	{
		return *(character->Get());
	}
}


TArray<FAiCharData> UVoxtaClient::GetAvailableAiCharactersCopy() const
{
	TArray<FAiCharData> returnArray;
	returnArray.Reserve(m_characterList.Num());

	for (const TUniquePtr<const FAiCharData>& character : m_characterList)
	{
		if (character.IsValid())
		{
			returnArray.Add(*character);
		}
	}

	return returnArray;
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
	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this, Arguments = arguments] (float DeltaTime)
		{
			if (m_currentState == VoxtaClientState::Disconnected ||
				m_currentState == VoxtaClientState::Terminated)
			{
				UE_LOGFMT(VoxtaLog, Log, "Tried to process a message with the connection already severed, "
					"skipping processing of remaining response data.");
			}
			else if (Arguments.IsEmpty() || Arguments[0].GetType() != FSignalRValue::EValueType::Object)
			{
				UE_LOGFMT(VoxtaLog, Error, "Received invalid message from server.");
			}
			else if (HandleResponse(Arguments[0].AsObject()))
			{
				UE_LOGFMT(VoxtaLog, Log, "VoxtaServer message handled successfully.");
			}
			else
			{
				UE_LOGFMT(VoxtaLog, Warning, "Response handler reported a failure, please check the logs to see "
					"what's wrong. Type: {0}", Arguments[0].AsObject()[EASY_STRING("$type")].AsString());
			}

			return false; // Return false to remove the ticker after it runs once
		}));
}

void UVoxtaClient::OnConnected()
{
	/** Wait on the next tick to run the responsehandling on the GameThread,
	 * instead of the background thread of the socket. */
	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this] (float DeltaTime)
		{
			UE_LOGFMT(VoxtaLog, Log, "VoxtaClient connected successfully");
			SendMessageToServer(VoxtaApiRequestHandler::GetAuthenticateRequestData());
			return false; // Return false to remove the ticker after it runs once
		})
	);
}

void UVoxtaClient::OnConnectionError(const FString& error)
{
	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this, Error = error] (float DeltaTime)
		{
			UE_LOGFMT(VoxtaLog, Error, "VoxtaClient connection has encountered error: {0}.", Error);
			Disconnect();
			return false; // Return false to remove the ticker after it runs once
		}));
}

void UVoxtaClient::OnClosed()
{
	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this] (float DeltaTime)
		{
			if (GetCurrentState() != VoxtaClientState::Terminated &&
				GetCurrentState() != VoxtaClientState::Disconnected)
			{
				UE_LOGFMT(VoxtaLog, Log, "VoxtaClient connection has been closed.");
				Disconnect();
			}
			return false; // Return false to remove the ticker after it runs once
		}));
}

void UVoxtaClient::SendMessageToServer(const FSignalRValue& message)
{
	if (m_hub != nullptr)
	{
		m_hub->Invoke(SEND_MESSAGE_EVENT_NAME, message).BindUObject(this, &UVoxtaClient::OnMessageSent);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "Could not send message as the SignalR hub was destroyed, are you running in a test environment?.");
	}
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
	bool (UVoxtaClient::* handler)(const T&), bool isSensitive)
{
	const T* derivedResponse = StaticCast<const T*>(response);
	if (derivedResponse)
	{
		if (isSensitive)
		{
			SENSITIVE_LOG1(VoxtaLog, Log, "{0}", logMessage)
		}
		else
		{
			UE_LOGFMT(VoxtaLog, Log, "{0}", logMessage);
		}
		return (this->*handler)(*derivedResponse);
	}
	return false;
}

bool UVoxtaClient::HandleResponse(const TMap<FString, FSignalRValue>& responseData)
{
	FString responseType = responseData[EASY_STRING("$type")].AsString();
	if (VoxtaApiResponseHandler::IGNORED_MESSAGE_TYPES.Contains(responseType))
	{
		UE_LOGFMT(VoxtaLog, Log, "Ignoring message of type: {0}", responseType);
		return true;
	}

	TUniquePtr<ServerResponseBase> response = VoxtaApiResponseHandler::GetResponseData(responseData);
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
				TEXT("Logged in successfully"), &UVoxtaClient::HandleWelcomeResponse, false);
		case CharacterList:
			return HandleResponseHelper<ServerResponseCharacterList>(response.Get(),
				TEXT("Fetched characters successfully"), &UVoxtaClient::HandleCharacterListResponse, false);
		case ChatStarted:
			return HandleResponseHelper<ServerResponseChatStarted>(response.Get(),
				TEXT("Chat started successfully"), &UVoxtaClient::HandleChatStartedResponse, false);
		case ChatMessage:
			return HandleResponseHelper<ServerResponseChatMessageBase>(response.Get(),
				TEXT("Chat Message received successfully"), &UVoxtaClient::HandleChatMessageResponse, false);
		case ChatUpdate:
			return HandleResponseHelper<ServerResponseChatUpdate>(response.Get(),
				TEXT("Chat Update received successfully"), &UVoxtaClient::HandleChatUpdateResponse, false);
		case SpeechTranscription:
			return HandleResponseHelper<ServerResponseSpeechTranscription>(response.Get(),
				TEXT("Speech transcription update received successfully"),
				&UVoxtaClient::HandleSpeechTranscriptionResponse, false);
		case Error:
			return HandleResponseHelper<ServerResponseError>(response.Get(),
				TEXT("Error message received successfully"),
				&UVoxtaClient::HandleErrorResponse, false);
		case ContextUpdated:
			return HandleResponseHelper<ServerResponseContextUpdated>(response.Get(),
				TEXT("Context Updated message received successfully"),
				&UVoxtaClient::HandleContextUpdateResponse, false);
		case ChatClosed:
			return HandleResponseHelper<ServerResponseChatClosed>(response.Get(),
				TEXT("Chat closed successfully"), &UVoxtaClient::HandleChatClosedResponse, false);
		case ChatSessionError:
			return HandleResponseHelper<ServerResponseChatSessionError>(response.Get(),
				TEXT("Chat session error received successfully"), &UVoxtaClient::HandleChatSessionErrorResponse, false);
		case Configuration:
			return HandleResponseHelper<ServerResponseConfiguration>(response.Get(),
				TEXT("Configuration successfully"), &UVoxtaClient::HandleConfigurationResponse, false);
		default:
			UE_LOGFMT(VoxtaLog, Error, "No handler available for type a message of type: {0}", responseType);
			return false;
	}
}

bool UVoxtaClient::HandleWelcomeResponse(const ServerResponseWelcome& response)
{
	m_userData = MakeUnique<FUserCharData>(response.USER_DATA);
	m_mainAssistantId = response.ASSISTANT_ID;
	m_voxtaVersionData = MakeUnique<FVoxtaVersionData>(response.SERVER_VERSION, response.API_VERSION);

	if (m_voxtaVersionData->IsMatchingAPIVersion())
	{
		SENSITIVE_LOG1(VoxtaLog, Log, "API version is matching, Authenticated with Voxta Server. Welcome {0}! :D",
			m_userData->GetName());

		SetState(VoxtaClientState::Authenticated);
		SendMessageToServer(VoxtaApiRequestHandler::GetLoadCharactersListData());
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "API version is not matching, please use a VoxtaServer with API version {0}.",
			m_voxtaVersionData->GetCompatibleAPIVersion());
		Disconnect();
	}

	return true;
}

bool UVoxtaClient::HandleCharacterListResponse(const ServerResponseCharacterList& response)
{
	m_characterList.Empty();
	for (const FAiCharData& charElement : response.CHARACTERS)
	{
		m_characterList.Emplace(MakeUnique<FAiCharData>(charElement));
		VoxtaClientCharacterRegisteredEventNative.Broadcast(charElement);
		VoxtaClientCharacterRegisteredEvent.Broadcast(charElement);
	}
	SetState(VoxtaClientState::Idle);
	return true;
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

	if (!response.SERVICES.Contains(VoxtaServiceType::TextGen))
	{
		UE_LOGFMT(VoxtaLog, Error, "No valid TextGen service is active on the server. We cannot really do anything "
			"without this... aborting creation of chat session.");
		return false;
	}

	m_chatSession = MakeUnique<FChatSession>(chatCharacters, response.CHAT_ID,
		response.SESSION_ID, response.SERVICES, response.CONTEXT_TEXT);

	if (!response.SERVICES.Contains(VoxtaServiceType::TextToSpeech))
	{
		UE_LOGFMT(VoxtaLog, Log, "No valid TextToSpeech service is active on the server.");
	}

	if (!response.SERVICES.Contains(VoxtaServiceType::SpeechToText))
	{
		UE_LOGFMT(VoxtaLog, Log, "No valid SpeechToText service is active on the server.");
	}
	else
	{
		if (!m_voiceInput->IsInitialized())
		{
			m_voiceInput->InitializeSocket();
		}
		m_voiceInput->ConnectToCurrentChat();
	}

	SendMessageToServer(VoxtaApiRequestHandler::GetInspectorRequestData(m_chatSession->GetSessionId()));

	VoxtaClientChatSessionStartedEventNative.Broadcast(*m_chatSession.Get());
	VoxtaClientChatSessionStartedEvent.Broadcast(*m_chatSession.Get());
	SetState(VoxtaClientState::GeneratingReply);
	return true;
}

bool UVoxtaClient::HandleChatMessageResponse(const ServerResponseChatMessageBase& response)
{
	if (!m_chatSession.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Error, "Received a chat message, but there's no ongoing chat, "
			"a critical service was likely not available.");
		return false;
	}

	using enum ServerResponseChatMessageBase::ChatMessageType;
	switch (response.MESSAGE_TYPE)
	{
		case MessageStart:
		{
			const ServerResponseChatMessageStart* derivedResponse =
				StaticCast<const ServerResponseChatMessageStart*>(&response);
			m_chatSession->AddChatMessage(FChatMessage(derivedResponse->MESSAGE_ID, derivedResponse->SENDER_ID));

			UE_LOGFMT(VoxtaLog, Log, "Registered start of message with id: {0}", derivedResponse->MESSAGE_ID);
			break;
		}
		case MessageChunk:
		{
			const ServerResponseChatMessageChunk* derivedResponse =
				StaticCast<const ServerResponseChatMessageChunk*>(&response);
			FChatMessage* chatMessage = m_chatSession->GetChatMessageById(derivedResponse->MESSAGE_ID);

			if (chatMessage)
			{
				// Add a space before the text when appending it to the previous. As VoxtaServer ends the last sentence
				// of a chunk directly after the period, without an extra space.
				chatMessage->TryAppendMoreContent(chatMessage->GetTextContent().IsEmpty()
					? derivedResponse->MESSAGE_TEXT
					: TEXT(" ") + derivedResponse->MESSAGE_TEXT, derivedResponse->AUDIO_URL_PATH);
				UE_LOGFMT(VoxtaLog, Log, "Updated message contents of message with id: {0}", derivedResponse->MESSAGE_ID);
			}
			else
			{
				SENSITIVE_LOG2(VoxtaLog, Warning, "Received a messageChunk without already having the start of the message. "
					"messageId: {0}, content: {1}", derivedResponse->MESSAGE_ID, derivedResponse->MESSAGE_TEXT)
			}
			break;
		}
		case MessageEnd:
		{
			const ServerResponseChatMessageEnd* derivedResponse = StaticCast<const ServerResponseChatMessageEnd*>(&response);
			FChatMessage* rawPtr = m_chatSession->GetChatMessageById(derivedResponse->MESSAGE_ID);
			if (!rawPtr)
			{
				SENSITIVE_LOG1(VoxtaLog, Error, "Received replyEnd without matching start. messageId: {0}", derivedResponse->MESSAGE_ID);
				break;
			}
			rawPtr->MarkComplete();
			const FChatMessage* chatMessage = rawPtr;

			if (chatMessage)
			{
				const TUniquePtr<const FAiCharData>* character = GetAiCharacterDataById(derivedResponse->SENDER_ID);

				if (character != nullptr && character->IsValid())
				{
					SENSITIVE_LOG3(VoxtaLog, Log, "Message with id: {0} marked as complete. Speaker: {1} Contents: {2}",
						derivedResponse->MESSAGE_ID, character->Get()->GetName(), chatMessage->GetTextContent())

					auto playbackHandler = m_registeredCharacterAudioPlaybackComps.Find(character->Get()->GetId());
					if (playbackHandler != nullptr)
					{
						SetState(VoxtaClientState::AudioPlayback);
						playbackHandler->Get()->PlaybackMessage(*character->Get(), *chatMessage);
					}
					else
					{
						if (chatMessage->GetAudioUrls().Num() > 0)
						{
							if (IsGlobalAudioFallbackActive())
							{
								SetState(VoxtaClientState::AudioPlayback);
								m_globalAudioPlaybackComp->GetGlobalPlaybackComponent()->PlaybackMessage(*character->Get(), *chatMessage);
							}
							else
							{
								// Generated audio but configuration indicates playback is not desired.
								NotifyAudioPlaybackComplete(chatMessage->GetMessageId());
							}
						}
						else
						{
							SetState(VoxtaClientState::WaitingForUserResponse);
						}
					}
					VoxtaClientCharMessageAddedEventNative.Broadcast(*character->Get(), *chatMessage);
					VoxtaClientCharMessageAddedEvent.Broadcast(*character->Get(), *chatMessage);
				}
				else
				{
					SENSITIVE_LOG1(VoxtaLog, Warning, "Received a messageEnd without already having the start of the message. "
						"messageId: {0}", derivedResponse->MESSAGE_ID)
				}
			}
			else
			{
				SENSITIVE_LOG1(VoxtaLog, Warning, "Received a messageEnd without already having the start of the message. "
					"messageId: {0}", derivedResponse->MESSAGE_ID)
			}
			break;
		}
		case MessageCancelled:
		{
			const TArray<FChatMessage>& messages = m_chatSession->GetChatMessages();

			const ServerResponseChatMessageCancelled* derivedResponse =
				StaticCast<const ServerResponseChatMessageCancelled*>(&response);
			int index = messages.IndexOfByPredicate([derivedResponse] (const FChatMessage& InItem)
				{
					return InItem.GetMessageId() == derivedResponse->MESSAGE_ID;
				});

			if (index != INDEX_NONE)
			{
				UE_LOGFMT(VoxtaLog, Log, "Message with id: {0} marked as cancelled, removing it from the history.",
					derivedResponse->MESSAGE_ID);

				VoxtaClientCharMessageRemovedEventNative.Broadcast(messages[index]);
				VoxtaClientCharMessageRemovedEvent.Broadcast(messages[index]);
				m_chatSession->RemoveChatMessage(derivedResponse->MESSAGE_ID);
			}
			else
			{
				UE_LOGFMT(VoxtaLog, Log, "Message with id: {0} marked as cancelled, but it was not found in the history?",
					derivedResponse->MESSAGE_ID);
			}
		}
	}
	return true;
}

bool UVoxtaClient::HandleChatUpdateResponse(const ServerResponseChatUpdate& response)
{
	if (!m_chatSession.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Error, "Received a chat update, but there's no ongoing chat, this should never happen.");
		return false;
	}

	if (response.SESSION_ID == m_chatSession->GetSessionId())
	{
		if (m_chatSession->GetChatMessageById(response.MESSAGE_ID) != nullptr)
		{
			SENSITIVE_LOG3(VoxtaLog, Error, "Recieved a chat update but a message with that id already exists, "
				"let me know if this ever triggers as it has no implementation. Sender: {0} MessageId {1} Content: {2}",
				response.SENDER_ID, response.MESSAGE_ID, response.TEXT_CONTENT);
			return false;
		}
		else
		{
			if (m_userData.Get()->GetId() != response.SENDER_ID)
			{
				SENSITIVE_LOG2(VoxtaLog, Error, "Recieved chat update for an ai character, "
					"let me know if this ever triggers cuz it seemed to be only user-specific. Sender: {0} Content: {1}",
					response.SENDER_ID, response.TEXT_CONTENT);
				return false;
			}

			SENSITIVE_LOG2(VoxtaLog, Log, "Adding user-message to history due to update from VoxtaServer. "
				"MessageId {0} Content: {1}", response.MESSAGE_ID, response.TEXT_CONTENT);

			FChatMessage message = FChatMessage(response.MESSAGE_ID, response.SENDER_ID);
			message.TryAppendMoreContent(response.TEXT_CONTENT, FString());
			message.MarkComplete();
			m_chatSession->AddChatMessage(MoveTemp(message));

			// we want a pointer after it's moved to the heap, just for safety.
			const FChatMessage* chatMessage = m_chatSession->GetChatMessageById(response.MESSAGE_ID);
			VoxtaClientCharMessageAddedEventNative.Broadcast(*m_userData.Get(), *chatMessage);
			VoxtaClientCharMessageAddedEvent.Broadcast(*m_userData.Get(), *chatMessage);
		}
	}
	else
	{
		SENSITIVE_LOG3(VoxtaLog, Warning, "Recieved chat update for a different session? This should never happen, I think. "
			"SessionId: {0} SenderId: {1} Content: {2}", response.SESSION_ID, response.SENDER_ID, response.TEXT_CONTENT)
	}
	return true;
}

bool UVoxtaClient::HandleSpeechTranscriptionResponse(const ServerResponseSpeechTranscription& response)
{
	using enum ServerResponseSpeechTranscription::TranscriptionState;
	switch (response.TRANSCRIPTION_STATE)
	{
		case Partial:
			SENSITIVE_LOG1(VoxtaLog, Log, "Received update for partial speech transcription. Current version of "
				"transcription: {0} ", response.TRANSCRIBED_SPEECH)

			VoxtaClientSpeechTranscribedPartialEventNative.Broadcast(response.TRANSCRIBED_SPEECH);
			VoxtaClientSpeechTranscribedPartialEvent.Broadcast(response.TRANSCRIBED_SPEECH);
			break;
		case End:
			if (m_currentState == VoxtaClientState::WaitingForUserResponse)
			{
				SENSITIVE_LOG1(VoxtaLog, Log, "Received finalized version of speech transcription: {0}",
					response.TRANSCRIBED_SPEECH)

				VoxtaClientSpeechTranscribedCompleteEventNative.Broadcast(response.TRANSCRIBED_SPEECH);
				VoxtaClientSpeechTranscribedCompleteEvent.Broadcast(response.TRANSCRIBED_SPEECH);
				SendUserInput(response.TRANSCRIBED_SPEECH);
			}
			// no logging for else, as Transcription sometimes sends the final version more than once, not sure why.
			break;
		case Cancelled:
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

bool UVoxtaClient::HandleContextUpdateResponse(const ServerResponseContextUpdated& response)
{
	if (!m_chatSession.IsValid())
	{
		SENSITIVE_LOG2(VoxtaLog, Warning, "Recieved a context update but there's no active chat session? This should not happen, "
			"skipping processing of response... Context: {0}, Session: {1}", response.CONTEXT_TEXT, response.SESSION_ID)
		return true;
	}

	if (m_chatSession->GetChatContext() == response.CONTEXT_TEXT)
	{
		SENSITIVE_LOG1(VoxtaLog, Log, "Received context update was identical to the current context: {0}", response.CONTEXT_TEXT)
		return true;
	}

	m_chatSession->UpdateContext(response.CONTEXT_TEXT);
	SENSITIVE_LOG1(VoxtaLog, Log, "Updated context of the chat session to: {0}", m_chatSession->GetChatContext());
	VoxtaClientChatContextUpdatedEventNative.Broadcast(m_chatSession->GetChatContext().GetData());
	VoxtaClientChatContextUpdatedEvent.Broadcast(m_chatSession->GetChatContext().GetData());
	return true;
}

bool UVoxtaClient::HandleChatClosedResponse(const ServerResponseChatClosed& response)
{
	if (!m_chatSession.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Warning, "Recieved a chat closed response, but there's no active chat session? This should not happen, "
			"skipping processing of response... Chat: {0}, Session: {1}", response.CHAT_ID, response.SESSION_ID);
		return true;
	}

	SetState(VoxtaClientState::Idle);
	StopChatInternal();
	UE_LOGFMT(VoxtaLog, Log, "Released ongoing chat, VoxtaClient returning back to idle");

	return true;
}

bool UVoxtaClient::HandleChatSessionErrorResponse(const ServerResponseChatSessionError& response)
{
	if (!m_chatSession.IsValid())
	{
		UE_LOGFMT(VoxtaLog, Log, "Recieved a chatSessionError but no session was active. Message: {0}, ChatSessionId: {1}", response.ERROR_MESSAGE, response.ERROR_CHAT_SESSION_ID);
		return true;
	}
	UE_LOGFMT(VoxtaLog, Error, "Recieved a chatSessionError, unsure how to proceed. "
		"Message: {0}, ChatSessionId: {1}, Retry: {2}", response.ERROR_MESSAGE, response.ERROR_CHAT_SESSION_ID, response.ERROR_RETRY);

	return true;
}

bool UVoxtaClient::HandleConfigurationResponse(const ServerResponseConfiguration& response)
{
	UE_LOGFMT(VoxtaLog, Log, "Todo services stuff");
	return true;
}

void UVoxtaClient::StopChatInternal()
{
	if (!m_chatSession.IsValid())
	{
		return;
	}

	m_voiceInput->DisconnectFromChat();
	VoxtaClientChatSessionStoppedEventNative.Broadcast(*m_chatSession.Get());
	VoxtaClientChatSessionStoppedEvent.Broadcast(*m_chatSession.Get());
	m_chatSession.Reset();
}

const TUniquePtr<const FAiCharData>* UVoxtaClient::GetAiCharacterDataById(const FGuid& charId) const
{
	return m_characterList.FindByPredicate([&charId] (const TUniquePtr<const FAiCharData>& inItem)
		{
			return inItem->GetId() == charId;
		});
}

void UVoxtaClient::SetState(VoxtaClientState newState)
{
	UE_LOGFMT(VoxtaLog, Log, "Marking the current VoxtaClient state as: {0}", UEnum::GetValueAsString(newState));

	m_currentState = newState;
	VoxtaClientStateChangedEventNative.Broadcast(m_currentState);
	VoxtaClientStateChangedEvent.Broadcast(m_currentState);
}

AVoxtaGlobalAudioPlaybackHolder* UVoxtaClient::GetOrCreateGlobalAudioFallbackInternal()
{
	if (m_globalAudioPlaybackComp == nullptr)
	{
		if (UWorld* World = GetWorld())
		{
			m_globalAudioPlaybackComp = World->SpawnActor<AVoxtaGlobalAudioPlaybackHolder>();
			if (m_globalAudioPlaybackComp)
			{
				globalAudioPlaybackHandle = m_globalAudioPlaybackComp->GetGlobalPlaybackComponent()->VoxtaMessageAudioPlaybackFinishedEventNative.AddUObject(this, &UVoxtaClient::NotifyAudioPlaybackComplete);
			}
			else
			{
				UE_LOGFMT(VoxtaLog, Warning, "Failed to spawn AVoxtaGlobalAudioPlaybackHolder, global audio fallback disabled.");
			}
		}
		else
		{
			UE_LOGFMT(VoxtaLog, Warning, "GetWorld() returned null, cannot create global audio playback holder.");
		}
	}
	return m_globalAudioPlaybackComp;
}