// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaPayloads.generated.h"

// The structs in this file map 1:1 to the data structures that the VoxtaServer expects in requests/responses
// These should be kept exclusive for communication through the SignalR socket, and should not be used by any other system.

// ====================
// SHARED & NESTED TYPES
// ====================

USTRUCT()
struct FVoxtaPayload_EntityData
{
	GENERATED_BODY()

	UPROPERTY()
	FString id;
	UPROPERTY()
	FString name;
};

USTRUCT()
struct FVoxtaPayload_CharacterData
{
	GENERATED_BODY()

	UPROPERTY()
	FString id;
	UPROPERTY()
	FString name;
	UPROPERTY()
	FString creatorNotes;
	UPROPERTY()
	bool explicitContent = false;
	UPROPERTY()
	bool favorite = false;
	UPROPERTY()
	FString thumbnailUrl;
	UPROPERTY()
	FString packageId;
	UPROPERTY()
	FString packageName;
};

USTRUCT()
struct FVoxtaPayload_AudioCapabilities
{
	GENERATED_BODY()

	UPROPERTY()
	FString audioInput = TEXT("WebSocketStream");
	UPROPERTY()
	FString audioOutput = TEXT("Url");
	UPROPERTY()
	TArray<FString> acceptedAudioContentTypes = { TEXT("audio/x-wav") };
};

USTRUCT()
struct FVoxtaPayload_ContextData
{
	GENERATED_BODY()

	UPROPERTY()
	FString contextKey;
	UPROPERTY()
	FString text;
};

USTRUCT()
struct FVoxtaPayload_ServiceEntryData
{
	GENERATED_BODY()

	UPROPERTY()
	FString serviceLabel;
	UPROPERTY()
	FString id;
	UPROPERTY()
	bool enabled;
};

USTRUCT()
struct FVoxtaPayload_ServiceGroupData
{
	GENERATED_BODY()

	UPROPERTY()
	FString defaultServiceId;
};

USTRUCT()
struct FVoxtaPayload_Services
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVoxtaPayload_ServiceEntryData> textGen;
	UPROPERTY()
	TArray<FVoxtaPayload_ServiceEntryData> speechToText;
	UPROPERTY()
	TArray<FVoxtaPayload_ServiceEntryData> textToSpeech;
	UPROPERTY()
	TArray<FVoxtaPayload_ServiceEntryData> actionInference;
};

USTRUCT()
struct FVoxtaPayload_ConfigurationService
{
	GENERATED_BODY()

	UPROPERTY()
	FString defaultServiceId;
	UPROPERTY()
	bool enabled;
};

USTRUCT()
struct FVoxtaPayload_ConfigurationServices
{
	GENERATED_BODY()

	UPROPERTY()
	FVoxtaPayload_ConfigurationService textGen;
	UPROPERTY()
	FVoxtaPayload_ConfigurationService speechToText;
	UPROPERTY()
	FVoxtaPayload_ConfigurationService textToSpeech;
	UPROPERTY()
	FVoxtaPayload_ConfigurationService actionInference;
};

USTRUCT()
struct FVoxtaPayload_Configuration
{
	GENERATED_BODY()

	UPROPERTY()
	FString id;
	UPROPERTY()
	FString name;
	UPROPERTY()
	bool isDefault;
	UPROPERTY()
	FVoxtaPayload_ConfigurationServices services;
};

// ====================
// REQUEST PAYLOADS
// ====================

USTRUCT()
struct FVoxtaPayload_AuthenticateRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("authenticate");
	UPROPERTY()
	FName client = TEXT("UnrealVoxta");
	UPROPERTY()
	FName clientVersion = TEXT("0.1.2a");
	UPROPERTY()
	TArray<FString> scope = { TEXT("role:app") };
	UPROPERTY()
	FVoxtaPayload_AudioCapabilities capabilities;
};

USTRUCT()
struct FVoxtaPayload_LoadCharactersRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("loadCharactersList");
};

USTRUCT()
struct FVoxtaPayload_LoadScenariosRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("loadScenariosList");
};

USTRUCT()
struct FVoxtaPayload_LoadChatsRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("loadChatsList");
	UPROPERTY()
	FString characterId;
	UPROPERTY()
	FString scenarioId;
};

USTRUCT()
struct FVoxtaPayload_StartChatRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("startChat");
	UPROPERTY()
	FString contextKey;
	UPROPERTY()
	TArray<FString> characterIds;
	UPROPERTY()
	TArray<FVoxtaPayload_ContextData> contexts;
};

USTRUCT()
struct FVoxtaPayload_StopChatRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("stopChat");
	UPROPERTY()
	FString sessionId;
};

USTRUCT()
struct FVoxtaPayload_SendTextMessageRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("send");
	UPROPERTY()
	FString sessionId;
	UPROPERTY()
	FString text;
	UPROPERTY()
	bool doReply = true;
	UPROPERTY()
	bool doCharacterActionInference = false;
};

USTRUCT()
struct FVoxtaPayload_SpeechPlaybackStartRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("speechPlaybackStart");
	UPROPERTY()
	FString sessionId;
	UPROPERTY()
	FString messageId;
	UPROPERTY()
	int32 startIndex = 0;
	UPROPERTY()
	int32 endIndex = 0;
	UPROPERTY()
	FString duration;
};

USTRUCT()
struct FVoxtaPayload_SpeechPlaybackCompleteRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("speechPlaybackComplete");
	UPROPERTY()
	FString sessionId;
	UPROPERTY()
	FString messageId;
};

USTRUCT()
struct FVoxtaPayload_UpdateContextRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("updateContext");
	UPROPERTY()
	FString sessionId;
	UPROPERTY()
	FString contextKey;
	UPROPERTY()
	TArray<FVoxtaPayload_ContextData> contexts;
	UPROPERTY()
	TArray<FString> setFlags;
};

USTRUCT()
struct FVoxtaPayload_CharacterSpeechRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("characterSpeechRequest");
	UPROPERTY()
	FString sessionId;
	UPROPERTY()
	FString text;
};

USTRUCT()
struct FVoxtaPayload_RevertRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("revert");
	UPROPERTY()
	FString sessionId;
};

USTRUCT()
struct FVoxtaPayload_DeleteChatRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("deleteChat");
	UPROPERTY()
	FString chatId;
};

USTRUCT()
struct FVoxtaPayload_InspectRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("inspect");
	UPROPERTY()
	FString sessionId;
	UPROPERTY()
	bool enabled = false;
};

// ====================
// RESPONSE PAYLOADS
// ====================

USTRUCT()
struct FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString action;
	UPROPERTY()
	FString sessionId;
};

USTRUCT()
struct FVoxtaPayload_WelcomeResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FVoxtaPayload_EntityData user;
	UPROPERTY()
	FVoxtaPayload_EntityData assistant;
	UPROPERTY()
	FVoxtaPayload_EntityData favorite;
	UPROPERTY()
	FString voxtaServerVersion;
	UPROPERTY()
	FString apiVersion;
	UPROPERTY()
	FString registeredClientVersion;
};

USTRUCT()
struct FVoxtaPayload_ConfigurationResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FVoxtaPayload_Services services;

	UPROPERTY()
	TArray<FVoxtaPayload_Configuration> configurations;
};

USTRUCT()
struct FVoxtaPayload_CharacterListLoadedResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVoxtaPayload_CharacterData> characters;
};

USTRUCT()
struct FVoxtaPayload_ContextUpdatedResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVoxtaPayload_ContextData> contexts;
};

USTRUCT()
struct FVoxtaPayload_ChatStartedResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString chatId;
	UPROPERTY()
	FVoxtaPayload_EntityData user;
	UPROPERTY()
	TArray<FVoxtaPayload_EntityData> characters;
	UPROPERTY()
	FVoxtaPayload_Services services;
	UPROPERTY()
	TArray<FVoxtaPayload_ContextData> context;
};

USTRUCT()
struct FVoxtaPayload_ReplyStartResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString senderId;
};

USTRUCT()
struct FVoxtaPayload_ReplyChunkResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString senderId;
	UPROPERTY()
	int32 startIndex = 0;
	UPROPERTY()
	int32 endIndex = 0;
	UPROPERTY()
	FString text;
	UPROPERTY()
	FString audioUrl;
	UPROPERTY()
	bool isNarration = false;
};

USTRUCT()
struct FVoxtaPayload_ReplyEndResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString senderId;
};

USTRUCT()
struct FVoxtaPayload_ReplyCancelledResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString reason;
};

USTRUCT()
struct FVoxtaPayload_ChatUpdateResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString chatId;
	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString senderId;
	UPROPERTY()
	FString text;
};

USTRUCT()
struct FVoxtaPayload_ChatClosedResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString chatId;
};

USTRUCT()
struct FVoxtaPayload_SpeechRecognitionResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString text;
};

USTRUCT()
struct FVoxtaPayload_ErrorResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString message;
	UPROPERTY()
	FString details;
};

USTRUCT()
struct FVoxtaPayload_ChatSessionErrorResponse : public FVoxtaPayload_BaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	bool retry = false;
	UPROPERTY()
	FString message;
};
