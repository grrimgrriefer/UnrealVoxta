// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiHandler.h"
#include "JsonObjectConverter.h"

// ====================
// REQUEST BUILDERS
// ====================

FString FVoxtaApiHandler::BuildAuthenticatePayload(const FName& clientName, const FName& version)
{
	FVoxtaAuthenticateRequest requestPayload;
	requestPayload.client = clientName;
	requestPayload.clientVersion = version;

	FString outputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(requestPayload, outputJson))
	{
		return outputJson;
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaAuthenticateRequest."));
	return FString();
}
FString FVoxtaApiHandler::BuildLoadCharactersPayload()
{
	const FVoxtaLoadCharactersRequest requestPayload;
	FString outputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(requestPayload, outputJson))
	{
		return outputJson;
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaLoadCharactersRequest."));
	return FString();
}
FString FVoxtaApiHandler::BuildStartChatPayload(const FString& characterId)
{
	FVoxtaStartChatRequest requestPayload;
	requestPayload.characterId = characterId;

	FString outputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(requestPayload, outputJson))
	{
		return outputJson;
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaStartChatRequest."));
	return FString();
}
FString FVoxtaApiHandler::BuildStopChatPayload(const FString& sessionId)
{
	FVoxtaStopChatRequest requestPayload;
	requestPayload.sessionId = sessionId;

	FString outputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(requestPayload, outputJson))
	{
		return outputJson;
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaStopChatRequest."));
	return FString();
}
FString FVoxtaApiHandler::BuildSendTextMessagePayload(const FString& sessionId, const FString& text)
{
	FVoxtaSendTextMessageRequest requestPayload;
	requestPayload.sessionId = sessionId;
	requestPayload.text = text;

	FString outputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(requestPayload, outputJson))
	{
		return outputJson;
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaSendTextMessageRequest."));
	return FString();
}

// ====================
// RESPONSE PARSERS
// ====================

bool FVoxtaApiHandler::TryExtractAction(const FString& jsonString, FString& outAction)
{
	FVoxtaBaseResponse baseResponse;
	if (FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &baseResponse, 0, 0))
	{
		outAction = baseResponse.action;
		return !outAction.IsEmpty();
	}
	return false;
}
bool FVoxtaApiHandler::ParseWelcomeResponse(const FString& jsonString, FVoxtaWelcomeResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseCharacterListLoadedResponse(const FString& jsonString, FVoxtaCharacterListLoadedResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseContextUpdatedResponse(const FString& jsonString, FVoxtaContextUpdatedResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseChatStartedResponse(const FString& jsonString, FVoxtaChatStartedResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseReplyStartResponse(const FString& jsonString, FVoxtaReplyStartResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseReplyChunkResponse(const FString& jsonString, FVoxtaReplyChunkResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseReplyEndResponse(const FString& jsonString, FVoxtaReplyEndResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseReplyCancelledResponse(const FString& jsonString, FVoxtaReplyCancelledResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseChatUpdateResponse(const FString& jsonString, FVoxtaChatUpdateResponse& outResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &outResponse, 0, 0);
}
