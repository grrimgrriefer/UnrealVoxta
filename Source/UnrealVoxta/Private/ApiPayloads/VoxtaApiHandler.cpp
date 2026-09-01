// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiHandler.h"
#include "JsonObjectConverter.h"

// ====================
// REQUEST BUILDERS
// ====================

FString FVoxtaApiHandler::BuildAuthenticatePayload(const FString& ClientName, const FString& Version)
{
	FVoxtaAuthenticateRequest RequestPayload;
	RequestPayload.client = ClientName;
	RequestPayload.clientVersion = Version;

	FString OutputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(RequestPayload, OutputJson))
	{
		return OutputJson;
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaAuthenticateRequest."));
	return FString();
}
FString FVoxtaApiHandler::BuildLoadCharactersPayload()
{
	FVoxtaLoadCharactersRequest RequestPayload;
	FString OutputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(RequestPayload, OutputJson))
	{
		return OutputJson;
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaLoadCharactersRequest."));
	return FString();
}
FString FVoxtaApiHandler::BuildStartChatPayload(const FString& CharacterId)
{
	FVoxtaStartChatRequest RequestPayload;
	RequestPayload.characterId = CharacterId;

	FString OutputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(RequestPayload, OutputJson))
	{
		return OutputJson;
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaStartChatRequest."));
	return FString();
}
FString FVoxtaApiHandler::BuildStopChatPayload(const FString& SessionId)
{
	FVoxtaStopChatRequest RequestPayload;
	RequestPayload.sessionId = SessionId;

	FString OutputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(RequestPayload, OutputJson))
	{
		return OutputJson;
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaStopChatRequest."));
	return FString();
}
FString FVoxtaApiHandler::BuildSendTextMessagePayload(const FString& SessionId, const FString& Text)
{
	FVoxtaSendTextMessageRequest RequestPayload;
	RequestPayload.sessionId = SessionId;
	RequestPayload.text = Text;

	FString OutputJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(RequestPayload, OutputJson))
	{
		return OutputJson;
	}
	UE_LOG(LogTemp, Error, TEXT("[VoxtaApiHandler] Failed to serialize FVoxtaSendTextMessageRequest."));
	return FString();
}

// ====================
// RESPONSE PARSERS
// ====================

bool FVoxtaApiHandler::TryExtractAction(const FString& JsonString, FString& OutAction)
{
	FVoxtaBaseResponse BaseResponse;
	if (FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &BaseResponse, 0, 0))
	{
		OutAction = BaseResponse.action;
		return !OutAction.IsEmpty();
	}
	return false;
}
bool FVoxtaApiHandler::ParseWelcomeResponse(const FString& JsonString, FVoxtaWelcomeResponse& OutResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseCharacterListLoadedResponse(const FString& JsonString, FVoxtaCharacterListLoadedResponse& OutResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseContextUpdatedResponse(const FString& JsonString, FVoxtaContextUpdatedResponse& OutResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseChatStartedResponse(const FString& JsonString, FVoxtaChatStartedResponse& OutResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseReplyStartResponse(const FString& JsonString, FVoxtaReplyStartResponse& OutResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseReplyChunkResponse(const FString& JsonString, FVoxtaReplyChunkResponse& OutResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseReplyEndResponse(const FString& JsonString, FVoxtaReplyEndResponse& OutResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseReplyCancelledResponse(const FString& JsonString, FVoxtaReplyCancelledResponse& OutResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutResponse, 0, 0);
}
bool FVoxtaApiHandler::ParseChatUpdateResponse(const FString& JsonString, FVoxtaChatUpdateResponse& OutResponse)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutResponse, 0, 0);
}
