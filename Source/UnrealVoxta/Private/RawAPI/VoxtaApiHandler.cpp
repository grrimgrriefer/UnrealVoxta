// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiHandler.h"
#include "JsonObjectConverter.h"
#include "VoxtaPayloads.h"
#include "VoxtaResponseDispatcher.h"
#include "VoxtaSocketHandler.h"

const FName UVoxtaApiHandler::CLIENT_NAME = TEXT("UnrealVoxta");
const FName UVoxtaApiHandler::CLIENT_VERSION = TEXT("0.2.0");

void UVoxtaApiHandler::Initialize()
{
	m_voxtaSocketHandler = NewObject<UVoxtaSocketHandler>(this);
	m_voxtaSocketHandler->m_OnSocketConnected.AddUObject(this, &UVoxtaApiHandler::OnSocketConnected);
	m_voxtaSocketHandler->m_OnSocketConnectionError.AddUObject(this, &UVoxtaApiHandler::OnSocketConnectionError);
	m_voxtaSocketHandler->m_OnSocketClosed.AddUObject(this, &UVoxtaApiHandler::OnSocketClosed);
	m_voxtaSocketHandler->m_OnMessageReceived.AddUObject(this, &UVoxtaApiHandler::OnMessageReceived);

	RegisterResponseRoute<FVoxtaWelcomeResponse>(TEXT("welcome"));
	RegisterResponseRoute<FVoxtaCharacterListLoadedResponse>(TEXT("charactersListLoaded"));
	RegisterResponseRoute<FVoxtaCharacterListLoadedResponse>(TEXT("characterListLoaded"));
	RegisterResponseRoute<FVoxtaContextUpdatedResponse>(TEXT("contextUpdated"));
	RegisterResponseRoute<FVoxtaChatStartedResponse>(TEXT("chatStarted"));
	RegisterResponseRoute<FVoxtaReplyStartResponse>(TEXT("replyStart"));
	RegisterResponseRoute<FVoxtaReplyChunkResponse>(TEXT("replyChunk"));
	RegisterResponseRoute<FVoxtaReplyEndResponse>(TEXT("replyEnd"));
	RegisterResponseRoute<FVoxtaReplyCancelledResponse>(TEXT("replyCancelled"));
	RegisterResponseRoute<FVoxtaChatUpdateResponse>(TEXT("update"));
	RegisterResponseRoute<FVoxtaChatClosedResponse>(TEXT("chatClosed"));
	RegisterResponseRoute<FVoxtaSpeechRecognitionResponse>(TEXT("speechRecognitionPartial"));
	RegisterResponseRoute<FVoxtaSpeechRecognitionResponse>(TEXT("speechRecognitionEnd"));
}
void UVoxtaApiHandler::EstablishConnection(const FString& ipv4Address, int port) const
{
	m_voxtaSocketHandler->EstablishConnection(ipv4Address, port);
}
void UVoxtaApiHandler::Disconnect() const
{
	m_voxtaSocketHandler->Disconnect();
}
bool UVoxtaApiHandler::TrySendAuthenticatePayload(const FName& clientName, const FName& version) const
{
	FVoxtaAuthenticateRequest requestPayload;
	requestPayload.client = clientName;
	requestPayload.clientVersion = version;
	return TrySendPayloadInternal(requestPayload, TEXT("FVoxtaAuthenticateRequest"));
}
bool UVoxtaApiHandler::TrySendLoadCharactersPayload() const
{
	return TrySendPayloadInternal(FVoxtaLoadCharactersRequest(), TEXT("FVoxtaLoadCharactersRequest"));
}
bool UVoxtaApiHandler::TrySendStartChatPayload(const FString& characterId) const
{
	FVoxtaStartChatRequest requestPayload;
	requestPayload.characterIds.Add(characterId);
	return TrySendPayloadInternal(requestPayload, TEXT("FVoxtaStartChatRequest"));
}
bool UVoxtaApiHandler::TrySendStopChatPayload(const FString& sessionId) const
{
	FVoxtaStopChatRequest requestPayload;
	requestPayload.sessionId = sessionId;
	return TrySendPayloadInternal(requestPayload, TEXT("FVoxtaStopChatRequest"));
}
bool UVoxtaApiHandler::TrySendSendTextMessagePayload(const FString& sessionId, const FString& text) const
{
	FVoxtaSendTextMessageRequest requestPayload;
	requestPayload.sessionId = sessionId;
	requestPayload.text = text;
	return TrySendPayloadInternal(requestPayload, TEXT("FVoxtaSendTextMessageRequest"));
}
template <typename T>
TMulticastDelegate<void(const T&)>& UVoxtaApiHandler::OnResponse()
{
	const FName structName = T::StaticStruct()->GetFName();
	TSharedPtr<IVoxtaResponseDispatcher>& dispatcher = m_responseDispatchers.FindOrAdd(structName);
	if (!dispatcher.IsValid())
	{
		dispatcher = MakeShared<TVoxtaResponseDispatcher<T>>();
	}
	return StaticCastSharedPtr<TVoxtaResponseDispatcher<T>>(dispatcher)->m_delegate;
}
void UVoxtaApiHandler::OnSocketConnected()
{
	m_OnConnected.Broadcast();
}
void UVoxtaApiHandler::OnSocketConnectionError(const FString& error)
{
	m_OnConnectionError.Broadcast(error);
}
void UVoxtaApiHandler::OnSocketClosed()
{
	m_OnDisconnected.Broadcast();
}
void UVoxtaApiHandler::OnMessageReceived(const FString& jsonString)
{
	FString action;
	if (!TryExtractAction(jsonString, action))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoxtaApiHandler] Failed to extract action from message: %s"), *jsonString);
		return;
	}

	if (const TFunction<void(const FString&)>* route = m_responseRoutes.Find(action))
	{
		(*route)(jsonString);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("[VoxtaApiHandler] No route registered for action: %s"), *action);
	}
}
template <typename T>
bool UVoxtaApiHandler::TrySendPayloadInternal(const T& payload, const TCHAR* payloadName) const
{
	const TSharedPtr<FJsonObject> jsonObject = FJsonObjectConverter::UStructToJsonObject(payload);
	if (jsonObject.IsValid())
	{
		return m_voxtaSocketHandler->TrySendPayload(jsonObject.Get());
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize %s."), payloadName);
	return false;
}
template <typename T>
bool UVoxtaApiHandler::ParseResponseInternal(const FString& jsonString, T& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
template <typename T>
void UVoxtaApiHandler::RegisterResponseRoute(const FString& actionName)
{
	m_responseRoutes.Add(actionName, {
		[this](const FString& jsonString)
		{
			T response;
			if (FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &response, 0, 0))
			{
				BroadcastResponse<T>(response);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to deserialize %s"), *T::StaticStruct()->GetName());
			}
		}
	});
}
template <typename T>
void UVoxtaApiHandler::BroadcastResponse(const T& response)
{
	const FName structName = T::StaticStruct()->GetFName();
	if (const TSharedPtr<IVoxtaResponseDispatcher>* dispatcher = m_responseDispatchers.Find(structName))
	{
		if (dispatcher->IsValid())
		{
			StaticCastSharedPtr<TVoxtaResponseDispatcher<T>>(*dispatcher)->m_Delegate.Broadcast(response);
		}
	}
}
bool UVoxtaApiHandler::TryExtractAction(const FString& jsonString, FString& outAction)
{
	TSharedPtr<FJsonObject> jsonObject;
	const TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonString);
	if (FJsonSerializer::Deserialize(reader, jsonObject) && jsonObject.IsValid())
	{
		if (jsonObject->TryGetStringField(TEXT("action"), outAction) ||
			jsonObject->TryGetStringField(TEXT("$type"), outAction))
		{
			return !outAction.IsEmpty();
		}
	}
	return false;
}
