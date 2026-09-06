// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiHandler.h"
#include "JsonObjectConverter.h"
#include "VoxtaSocketHandler.h"

const FName UVoxtaApiHandler::CLIENT_NAME = TEXT("UnrealVoxta");
const FName UVoxtaApiHandler::CLIENT_VERSION = TEXT("0.2.0");

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

	FString outputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(requestPayload, outputJson))
	{
		return m_voxtaSocketHandler->TrySendPayload(outputJson);
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaAuthenticateRequest."));
	return false;
}
bool UVoxtaApiHandler::TrySendLoadCharactersPayload() const
{
	const FVoxtaLoadCharactersRequest requestPayload;
	FString outputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(requestPayload, outputJson))
	{
		return m_voxtaSocketHandler->TrySendPayload(outputJson);
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaLoadCharactersRequest."));
	return false;
}
bool UVoxtaApiHandler::TrySendStartChatPayload(const FString& characterId) const
{
	FVoxtaStartChatRequest requestPayload;
	requestPayload.characterId = characterId;

	FString outputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(requestPayload, outputJson))
	{
		return m_voxtaSocketHandler->TrySendPayload(outputJson);
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaStartChatRequest."));
	return false;
}
bool UVoxtaApiHandler::TrySendStopChatPayload(const FString& sessionId) const
{
	FVoxtaStopChatRequest requestPayload;
	requestPayload.sessionId = sessionId;

	FString outputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(requestPayload, outputJson))
	{
		return m_voxtaSocketHandler->TrySendPayload(outputJson);
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaStopChatRequest."));
	return false;
}
bool UVoxtaApiHandler::TrySendSendTextMessagePayload(const FString& sessionId, const FString& text) const
{
	FVoxtaSendTextMessageRequest requestPayload;
	requestPayload.sessionId = sessionId;
	requestPayload.text = text;

	FString outputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(requestPayload, outputJson))
	{
		return m_voxtaSocketHandler->TrySendPayload(outputJson);
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaSendTextMessageRequest."));
	return false;
}

// ====================
// RESPONSE PARSERS
// ====================

bool UVoxtaApiHandler::TryExtractAction(const FString& jsonString, FString& outAction)
{
	FVoxtaBaseResponse baseResponse;
	if (FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &baseResponse, 0, 0))
	{
		outAction = baseResponse.action;
		return !outAction.IsEmpty();
	}
	return false;
}
bool UVoxtaApiHandler::ParseWelcomeResponse(const FString& jsonString, FVoxtaWelcomeResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool UVoxtaApiHandler::ParseCharacterListLoadedResponse(const FString& jsonString, FVoxtaCharacterListLoadedResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool UVoxtaApiHandler::ParseContextUpdatedResponse(const FString& jsonString, FVoxtaContextUpdatedResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool UVoxtaApiHandler::ParseChatStartedResponse(const FString& jsonString, FVoxtaChatStartedResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool UVoxtaApiHandler::ParseReplyStartResponse(const FString& jsonString, FVoxtaReplyStartResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool UVoxtaApiHandler::ParseReplyChunkResponse(const FString& jsonString, FVoxtaReplyChunkResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool UVoxtaApiHandler::ParseReplyEndResponse(const FString& jsonString, FVoxtaReplyEndResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool UVoxtaApiHandler::ParseReplyCancelledResponse(const FString& jsonString, FVoxtaReplyCancelledResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool UVoxtaApiHandler::ParseChatUpdateResponse(const FString& jsonString, FVoxtaChatUpdateResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
