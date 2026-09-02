// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaPayloads.h"

/**
 * Helper class to serialize requests and deserialize responses using VoxtaPayloads.
 */
class FVoxtaApiHandler
{
public:
	// Request Builders
	static FString BuildAuthenticatePayload(const FName& clientName, const FName& version);
	static FString BuildLoadCharactersPayload();
	static FString BuildStartChatPayload(const FString& characterId);
	static FString BuildStopChatPayload(const FString& sessionId);
	static FString BuildSendTextMessagePayload(const FString& sessionId, const FString& text);

	// Response Parsers
	static bool TryExtractAction(const FString& jsonString, FString& outAction);
	static bool ParseWelcomeResponse(const FString& jsonString, FVoxtaWelcomeResponse& outResponse);
	static bool ParseCharacterListLoadedResponse(const FString& jsonString, FVoxtaCharacterListLoadedResponse& outResponse);
	static bool ParseContextUpdatedResponse(const FString& jsonString, FVoxtaContextUpdatedResponse& outResponse);
	static bool ParseChatStartedResponse(const FString& jsonString, FVoxtaChatStartedResponse& outResponse);
	static bool ParseReplyStartResponse(const FString& jsonString, FVoxtaReplyStartResponse& outResponse);
	static bool ParseReplyChunkResponse(const FString& jsonString, FVoxtaReplyChunkResponse& outResponse);
	static bool ParseReplyEndResponse(const FString& jsonString, FVoxtaReplyEndResponse& outResponse);
	static bool ParseReplyCancelledResponse(const FString& jsonString, FVoxtaReplyCancelledResponse& outResponse);
	static bool ParseChatUpdateResponse(const FString& jsonString, FVoxtaChatUpdateResponse& outResponse);
};
