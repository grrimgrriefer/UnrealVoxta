// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaPayloads.generated.h"

// ====================
// SHARED TYPES

USTRUCT()
struct  FVoxtaCharacterData
{
	GENERATED_BODY()

	UPROPERTY()
	FString id;
	UPROPERTY()
	FString name;
	UPROPERTY()
	FString creator;
	UPROPERTY()
	FString description;
};

// ====================
// REQUEST PAYLOADS

USTRUCT()
struct FVoxtaAuthenticateRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("authenticate");
	UPROPERTY()
	FName client;
	UPROPERTY()
	FName clientVersion;
};

USTRUCT()
struct FVoxtaLoadCharactersRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("loadCharacters");
};

USTRUCT()
struct FVoxtaStartChatRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName action = TEXT("startChat");
	UPROPERTY()
	FString characterId;
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
	FName action = TEXT("sendTextMessage");
	UPROPERTY()
	FString sessionId;
	UPROPERTY()
	FString text;
};

// ====================
// RESPONSE PAYLOADS
// ====================

USTRUCT()
struct  FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString action;
	UPROPERTY()
	FString sessionId;
};

USTRUCT()
struct  FVoxtaWelcomeResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString user;
	UPROPERTY()
	TArray<FVoxtaCharacterData> characters;
};

USTRUCT()
struct  FVoxtaCharacterListLoadedResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVoxtaCharacterData> characters;
};

USTRUCT()
struct  FVoxtaContextUpdatedResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString context;
};

USTRUCT()
struct  FVoxtaChatStartedResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString chatId;
	UPROPERTY()
	FString characterId;
};

USTRUCT()
struct  FVoxtaReplyStartResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString senderId;
};

USTRUCT()
struct  FVoxtaReplyChunkResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString senderId;
	UPROPERTY()
	FString text;
};

USTRUCT()
struct  FVoxtaReplyEndResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString senderId;
};

USTRUCT()
struct  FVoxtaReplyCancelledResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString reason;
};

USTRUCT()
struct  FVoxtaChatUpdateResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString chatId;
};
