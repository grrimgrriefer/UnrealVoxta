// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.De

#include "VoxtaApiHandler.h"
#include "JsonObjectConverter.h"
#include "VoxtaGlobals.h"
#include "VoxtaPayloads.h"
#include "VoxtaResponseDispatcher.h"
#include "VoxtaSocketHandler.h"

const FName UVoxtaApiHandler::CLIENT_NAME = TEXT("UnrealVoxta");
const FName UVoxtaApiHandler::CLIENT_VERSION = TEXT("0.2.0");
const FName UVoxtaApiHandler::COMPATIBLE_SERVER_VERSION = TEXT("2025-11");
const FName UVoxtaApiHandler::VOXTA_CONTEXT_KEY = TEXT("UnrealVoxta - SimpleChat");

void UVoxtaApiHandler::Initialize()
{
	m_voxtaSocketHandler = NewObject<UVoxtaSocketHandler>(this);
	m_voxtaSocketHandler->m_OnSocketConnected.AddUObject(this, &UVoxtaApiHandler::OnSocketConnected);
	m_voxtaSocketHandler->m_OnSocketConnectionError.AddUObject(this, &UVoxtaApiHandler::OnSocketConnectionError);
	m_voxtaSocketHandler->m_OnSocketClosed.AddUObject(this, &UVoxtaApiHandler::OnSocketClosed);
	m_voxtaSocketHandler->m_OnSocketMessageReceived.AddUObject(this, &UVoxtaApiHandler::OnMessageReceived);

	RegisterResponseRoute<FVoxtaPayload_WelcomeResponse>(TEXT("welcome"));
	RegisterResponseRoute<FVoxtaPayload_ConfigurationResponse>(TEXT("configuration"));
	RegisterResponseRoute<FVoxtaPayload_CharacterListLoadedResponse>(TEXT("charactersListLoaded"));
	RegisterResponseRoute<FVoxtaPayload_ContextUpdatedResponse>(TEXT("contextUpdated"));
	RegisterResponseRoute<FVoxtaPayload_ChatStartedResponse>(TEXT("chatStarted"));
	RegisterResponseRoute<FVoxtaPayload_ReplyStartResponse>(TEXT("replyStart"));
	RegisterResponseRoute<FVoxtaPayload_ReplyChunkResponse>(TEXT("replyChunk"));
	RegisterResponseRoute<FVoxtaPayload_ReplyEndResponse>(TEXT("replyEnd"));
	RegisterResponseRoute<FVoxtaPayload_ReplyCancelledResponse>(TEXT("replyCancelled"));
	RegisterResponseRoute<FVoxtaPayload_ChatUpdateResponse>(TEXT("update"));
	RegisterResponseRoute<FVoxtaPayload_ChatClosedResponse>(TEXT("chatClosed"));
	RegisterResponseRoute<FVoxtaPayload_SpeechRecognitionResponse>(TEXT("speechRecognitionPartial"));
	RegisterResponseRoute<FVoxtaPayload_SpeechRecognitionResponse>(TEXT("speechRecognitionEnd"));
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
	FVoxtaPayload_AuthenticateRequest requestPayload;
	requestPayload.client = clientName;
	requestPayload.clientVersion = version;
	return TrySendPayloadInternal(requestPayload, TEXT("FVoxtaAuthenticateRequest"));
}
bool UVoxtaApiHandler::TrySendLoadCharactersPayload() const
{
	return TrySendPayloadInternal(FVoxtaPayload_LoadCharactersRequest(), TEXT("FVoxtaLoadCharactersRequest"));
}
bool UVoxtaApiHandler::TrySendStartChatPayload(const FString& characterId, const FString& chatContext) const
{
	FVoxtaPayload_ContextData contextData;
	contextData.text = chatContext;

	FVoxtaPayload_StartChatRequest requestPayload;
	requestPayload.characterIds.Add(characterId);
	requestPayload.contextKey = VOXTA_CONTEXT_KEY;
	requestPayload.contexts.Add(contextData);
	return TrySendPayloadInternal(requestPayload, TEXT("FVoxtaStartChatRequest"));
}
bool UVoxtaApiHandler::TrySendStopChatPayload(const FString& sessionId) const
{
	FVoxtaPayload_StopChatRequest requestPayload;
	requestPayload.sessionId = sessionId;
	return TrySendPayloadInternal(requestPayload, TEXT("FVoxtaStopChatRequest"));
}
bool UVoxtaApiHandler::TrySendSendTextMessagePayload(const FString& sessionId, const FString& text) const
{
	FVoxtaPayload_SendTextMessageRequest requestPayload;
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
    return StaticCastSharedPtr<TVoxtaResponseDispatcher<T>>(dispatcher)->m_Delegate;
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
void UVoxtaApiHandler::OnMessageReceived(const TSharedPtr<FJsonObject>& jsonObject)
{
	ensureAlways(jsonObject.IsValid());

	FString action;
	if (!jsonObject->TryGetStringField(TEXT("$type"), action) &&
		!jsonObject->TryGetStringField(TEXT("action"), action))
	{
		UE_LOGFMT(LogVoxta, Error, "[{0}] Failed to extract action from message: {1}", StaticClass()->GetName());
		return;
	}

	if (const TFunction<void(const TSharedPtr<FJsonObject>&)>* route = m_responseRoutes.Find(action))
	{
		UE_LOGFMT(LogVoxta, Verbose, "[{0}] Calling response handler for action {1} ...", StaticClass()->GetName(), action);
		(*route)(jsonObject);
	}
	else
	{
		UE_LOGFMT(LogVoxta, Warning, "[{0}] No route registered for action: {1}", StaticClass()->GetName(), action);
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
	UE_LOGFMT(LogVoxta, Error, "[{0}] Failed to serialize {1}.", StaticClass()->GetName(), payloadName);
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
	m_responseRoutes.Add(actionName, [this](const TSharedPtr<FJsonObject>& jsonObject)
	{
		T response;
		if (FJsonObjectConverter::JsonObjectToUStruct(jsonObject.ToSharedRef(), &response))
		{
			UE_LOGFMT(LogVoxta, Verbose, "[{0}] Broadcasting response...", StaticClass()->GetName());
			BroadcastResponse<T>(response);
		}
		else
		{
			UE_LOGFMT(LogVoxta, Error,"[{0}] Failed to deserialize {1}", StaticClass()->GetName(), *T::StaticStruct()->GetName());
		}
	});
}
template <typename T>
void UVoxtaApiHandler::BroadcastResponse(const T& response)
{
	const FName structName = T::StaticStruct()->GetFName();
	const TSharedPtr<IVoxtaResponseDispatcher>* dispatcher = m_responseDispatchers.Find(structName);

	if (!dispatcher || !dispatcher->IsValid())
	{
		UE_LOGFMT(LogVoxta, Warning, "[{0}] No listeners registered for response {1}.", StaticClass()->GetName(), structName.ToString());
		return;
	}

	UE_LOGFMT(LogVoxta, Verbose, "[{0}] Dispatching response...", StaticClass()->GetName());
	StaticCastSharedPtr<TVoxtaResponseDispatcher<T>>(*dispatcher)->m_Delegate.Broadcast(response);
}
