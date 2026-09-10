// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaPayloads.generated.h"

// ====================
// SHARED & NESTED TYPES
// ====================

USTRUCT()
struct FVoxtaUserData
{
	GENERATED_BODY()

	UPROPERTY()
	FString id;
	UPROPERTY()
	FString name;
};

USTRUCT()
struct FVoxtaCharacterData
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
struct FVoxtaAudioCapabilities
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
struct FVoxtaContextData
{
	GENERATED_BODY()

	UPROPERTY()
	FString contextKey;
	UPROPERTY()
	FString text;
};

USTRUCT()
struct FVoxtaServiceEntryData
{
	GENERATED_BODY()

	UPROPERTY()
	FString serviceName;
	UPROPERTY()
	FString serviceId;
};

USTRUCT()
struct FVoxtaServiceGroupData
{
	GENERATED_BODY()

	UPROPERTY()
	FString defaultServiceId;
	UPROPERTY()
	TArray<FVoxtaServiceEntryData> services;
};

USTRUCT()
struct FVoxtaServicesConfig
{
	GENERATED_BODY()

	UPROPERTY()
	FVoxtaServiceGroupData textGen;
	UPROPERTY()
	FVoxtaServiceGroupData speechToText;
	UPROPERTY()
	FVoxtaServiceGroupData textToSpeech;
	UPROPERTY()
	FVoxtaServiceGroupData actionInference;
};

// ====================
// REQUEST PAYLOADS
// ====================

USTRUCT()
struct FVoxtaAuthenticateRequest
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
	FVoxtaAudioCapabilities capabilities;
};

USTRUCT()
struct FVoxtaLoadCharactersRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("loadCharactersList");
};

USTRUCT()
struct FVoxtaLoadScenariosRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("loadScenariosList");
};

USTRUCT()
struct FVoxtaLoadChatsRequest
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
struct FVoxtaStartChatRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("startChat");
	UPROPERTY()
	FString contextKey;
	UPROPERTY()
	TArray<FString> characterIds;
	UPROPERTY()
	TArray<FVoxtaContextData> contexts;
};

USTRUCT()
struct FVoxtaStopChatRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("stopChat");
	UPROPERTY()
	FString sessionId;
};

USTRUCT()
struct FVoxtaSendTextMessageRequest
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
struct FVoxtaSpeechPlaybackStartRequest
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
struct FVoxtaSpeechPlaybackCompleteRequest
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
struct FVoxtaUpdateContextRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("updateContext");
	UPROPERTY()
	FString sessionId;
	UPROPERTY()
	FString contextKey;
	UPROPERTY()
	TArray<FVoxtaContextData> contexts;
	UPROPERTY()
	TArray<FString> setFlags;
};

USTRUCT()
struct FVoxtaCharacterSpeechRequest
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
struct FVoxtaRevertRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("revert");
	UPROPERTY()
	FString sessionId;
};

USTRUCT()
struct FVoxtaDeleteChatRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("deleteChat");
	UPROPERTY()
	FString chatId;
};

USTRUCT()
struct FVoxtaInspectRequest
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
struct FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString action;
	UPROPERTY()
	FString sessionId;
};

USTRUCT()
struct FVoxtaWelcomeResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FVoxtaUserData user;
	UPROPERTY()
	FVoxtaUserData assistant;
	UPROPERTY()
	FString voxtaServerVersion;
	UPROPERTY()
	FString apiVersion;
};

USTRUCT()
struct FVoxtaCharacterListLoadedResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVoxtaCharacterData> characters;
};

USTRUCT()
struct FVoxtaContextUpdatedResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVoxtaContextData> contexts;
};

USTRUCT()
struct FVoxtaChatStartedResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString chatId;
	UPROPERTY()
	FVoxtaUserData user;
	UPROPERTY()
	TArray<FVoxtaUserData> characters;
	UPROPERTY()
	FVoxtaServicesConfig services;
	UPROPERTY()
	TArray<FVoxtaContextData> context;
};

USTRUCT()
struct FVoxtaReplyStartResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString senderId;
};

USTRUCT()
struct FVoxtaReplyChunkResponse : public FVoxtaBaseResponse
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
struct FVoxtaReplyEndResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString senderId;
};

USTRUCT()
struct FVoxtaReplyCancelledResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString reason;
};

USTRUCT()
struct FVoxtaChatUpdateResponse : public FVoxtaBaseResponse
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
struct FVoxtaChatClosedResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString chatId;
};

USTRUCT()
struct FVoxtaSpeechRecognitionResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString text;
};

USTRUCT()
struct FVoxtaErrorResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString message;
	UPROPERTY()
	FString details;
};

USTRUCT()
struct FVoxtaChatSessionErrorResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	bool retry = false;
	UPROPERTY()
	FString message;
};

USTRUCT()
struct FVoxtaConfigurationResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FVoxtaServicesConfig services;
};
