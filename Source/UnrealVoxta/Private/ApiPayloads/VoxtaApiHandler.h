// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaPayloads.h"

/**
 * Helper class to serialize requests and deserialize responses using VoxtaPayloads.
 */
class UNREALVOXTA_API FVoxtaApiHandler
{
public:
	// Request Builders
	static FString BuildAuthenticatePayload(const FString& ClientName, const FString& Version);
	static FString BuildLoadCharactersPayload();
	static FString BuildStartChatPayload(const FString& CharacterId);
	static FString BuildStopChatPayload(const FString& SessionId);
	static FString BuildSendTextMessagePayload(const FString& SessionId, const FString& Text);

	// Response Parsers
	static bool TryExtractAction(const FString& JsonString, FString& OutAction);
	static bool ParseWelcomeResponse(const FString& JsonString, FVoxtaWelcomeResponse& OutResponse);
	static bool ParseCharacterListLoadedResponse(const FString& JsonString, FVoxtaCharacterListLoadedResponse& OutResponse);
	static bool ParseContextUpdatedResponse(const FString& JsonString, FVoxtaContextUpdatedResponse& OutResponse);
	static bool ParseChatStartedResponse(const FString& JsonString, FVoxtaChatStartedResponse& OutResponse);
	static bool ParseReplyStartResponse(const FString& JsonString, FVoxtaReplyStartResponse& OutResponse);
	static bool ParseReplyChunkResponse(const FString& JsonString, FVoxtaReplyChunkResponse& OutResponse);
	static bool ParseReplyEndResponse(const FString& JsonString, FVoxtaReplyEndResponse& OutResponse);
	static bool ParseReplyCancelledResponse(const FString& JsonString, FVoxtaReplyCancelledResponse& OutResponse);
	static bool ParseChatUpdateResponse(const FString& JsonString, FVoxtaChatUpdateResponse& OutResponse);
};
