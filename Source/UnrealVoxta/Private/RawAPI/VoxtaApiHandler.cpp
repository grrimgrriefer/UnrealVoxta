// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiHandler.h"
#include "JsonObjectConverter.h"
#include "VoxtaSocketHandler.h"

const FName UVoxtaApiHandler::CLIENT_NAME = TEXT("UnrealVoxta");
const FName UVoxtaApiHandler::CLIENT_VERSION = TEXT("0.2.0");

void UVoxtaApiHandler::Initialize()
{
	m_voxtaSocketHandler = NewObject<UVoxtaSocketHandler>(this);
	m_voxtaSocketHandler->m_OnSocketConnected.AddUObject(this, &UVoxtaApiHandler::OnSocketConnected);
	m_voxtaSocketHandler->m_OnSocketConnectionError.AddUObject(this, &UVoxtaApiHandler::OnSocketConnectionError);
	m_voxtaSocketHandler->m_OnSocketClosed.AddUObject(this, &UVoxtaApiHandler::OnSocketClosed);
}

// ====================
// REQUESTS
// ====================

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

// ====================
// SOCKET CALLBACKS
// ====================

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

// ====================
// HELPER TEMPLATES
// ====================

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

// ====================
// RESPONSE PARSERS
// ====================

bool UVoxtaApiHandler::TryExtractAction(const FString& jsonString, FString& outAction)
{
	FVoxtaBaseResponse baseResponse;
	if (ParseResponseInternal(jsonString, baseResponse))
	{
		outAction = baseResponse.action;
		return !outAction.IsEmpty();
	}
	return false;
}
bool UVoxtaApiHandler::ParseWelcomeResponse(const FString& jsonString, FVoxtaWelcomeResponse& outResponse)
{
	return ParseResponseInternal(jsonString, outResponse);
}
bool UVoxtaApiHandler::ParseCharacterListLoadedResponse(const FString& jsonString, FVoxtaCharacterListLoadedResponse& outResponse)
{
	return ParseResponseInternal(jsonString, outResponse);
}
bool UVoxtaApiHandler::ParseContextUpdatedResponse(const FString& jsonString, FVoxtaContextUpdatedResponse& outResponse)
{
	return ParseResponseInternal(jsonString, outResponse);
}
bool UVoxtaApiHandler::ParseChatStartedResponse(const FString& jsonString, FVoxtaChatStartedResponse& outResponse)
{
	return ParseResponseInternal(jsonString, outResponse);
}
bool UVoxtaApiHandler::ParseReplyStartResponse(const FString& jsonString, FVoxtaReplyStartResponse& outResponse)
{
	return ParseResponseInternal(jsonString, outResponse);
}
bool UVoxtaApiHandler::ParseReplyChunkResponse(const FString& jsonString, FVoxtaReplyChunkResponse& outResponse)
{
	return ParseResponseInternal(jsonString, outResponse);
}
bool UVoxtaApiHandler::ParseReplyEndResponse(const FString& jsonString, FVoxtaReplyEndResponse& outResponse)
{
	return ParseResponseInternal(jsonString, outResponse);
}
bool UVoxtaApiHandler::ParseReplyCancelledResponse(const FString& jsonString, FVoxtaReplyCancelledResponse& outResponse)
{
	return ParseResponseInternal(jsonString, outResponse);
}
bool UVoxtaApiHandler::ParseChatUpdateResponse(const FString& jsonString, FVoxtaChatUpdateResponse& outResponse)
{
	return ParseResponseInternal(jsonString, outResponse);
}
