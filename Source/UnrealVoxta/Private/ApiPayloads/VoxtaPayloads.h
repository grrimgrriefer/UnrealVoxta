// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaPayloads.generated.h"

// ====================
// SHARED TYPES

USTRUCT()
struct UNREALVOXTA_API FVoxtaCharacterData
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
	FString action = TEXT("authenticate");
	UPROPERTY()
	FString client = TEXT("UnrealEngine");
	UPROPERTY()
	FString clientVersion = TEXT("1.0.0");
};

USTRUCT()
struct FVoxtaLoadCharactersRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString action = TEXT("loadCharacters");
};

USTRUCT()
struct FVoxtaStartChatRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString action = TEXT("startChat");
	UPROPERTY()
	FString characterId;
};

USTRUCT()
struct FVoxtaStopChatRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString action = TEXT("stopChat");
	UPROPERTY()
	FString sessionId;
};

USTRUCT()
struct FVoxtaSendTextMessageRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString action = TEXT("sendTextMessage");
	UPROPERTY()
	FString sessionId;
	UPROPERTY()
	FString text;
};

// ====================
// RESPONSE PAYLOADS
// ====================

USTRUCT()
struct UNREALVOXTA_API FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString action;
	UPROPERTY()
	FString sessionId;
};

USTRUCT()
struct UNREALVOXTA_API FVoxtaWelcomeResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString user;
	UPROPERTY()
	TArray<FVoxtaCharacterData> characters;
};

USTRUCT()
struct UNREALVOXTA_API FVoxtaCharacterListLoadedResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVoxtaCharacterData> characters;
};

USTRUCT()
struct UNREALVOXTA_API FVoxtaContextUpdatedResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString context;
};

USTRUCT()
struct UNREALVOXTA_API FVoxtaChatStartedResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString chatId;
	UPROPERTY()
	FString characterId;
};

USTRUCT()
struct UNREALVOXTA_API FVoxtaReplyStartResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString senderId;
};

USTRUCT()
struct UNREALVOXTA_API FVoxtaReplyChunkResponse : public FVoxtaBaseResponse
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
struct UNREALVOXTA_API FVoxtaReplyEndResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString senderId;
};

USTRUCT()
struct UNREALVOXTA_API FVoxtaReplyCancelledResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;
	UPROPERTY()
	FString reason;
};

USTRUCT()
struct UNREALVOXTA_API FVoxtaChatUpdateResponse : public FVoxtaBaseResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString chatId;
};
