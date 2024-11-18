// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiRequestHandler.h"
#include "VoxtaDefines.h"
#include "SignalR/Public/SignalRValue.h"
#include "AiCharData.h"

FSignalRValue VoxtaApiRequestHandler::GetAuthenticateRequestData() const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("authenticate") },
		{ EASY_STRING("client"), SIGNALR_STRING("UnrealVoxta") },
		{ EASY_STRING("clientVersion"), SIGNALR_STRING("0.1.1a") },
		{ EASY_STRING("scope"), FSignalRValue(TArray<FSignalRValue> { SIGNALR_STRING("role:app") }) },
		{ EASY_STRING("capabilities"), FSignalRValue(TMap<FString, FSignalRValue> {
			{ EASY_STRING("audioInput"),  SIGNALR_STRING("WebSocketStream") },
			{ EASY_STRING("audioOutput"),  SIGNALR_STRING("Url") },
			{ EASY_STRING("acceptedAudioContentTypes"), FSignalRValue(TArray<FSignalRValue> {
				SIGNALR_STRING("audio/x-wav") }) }
		}) }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetLoadCharactersListData() const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("loadCharactersList") }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetLoadScenariosListData() const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("loadScenariosList") }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetLoadChatsListData(const FString& characterId, const FString& scenarioId) const
{
	TMap<FString, FSignalRValue> requestData = TMap<FString, FSignalRValue>{
		{ EASY_STRING("$type"), SIGNALR_STRING("loadChatsList") },
		{ EASY_STRING("characterId"), FSignalRValue(characterId) }
	};

	if (!scenarioId.IsEmpty())
	{
		requestData.Add(
		{
			EASY_STRING("scenarioId"), FSignalRValue(scenarioId)
		});
	}

	return FSignalRValue(requestData);
}

FSignalRValue VoxtaApiRequestHandler::GetStartChatRequestData(const FAiCharData* charData, const FString& context) const
{
	TMap<FString, FSignalRValue> requestData = TMap<FString, FSignalRValue>{
		{ EASY_STRING("$type"), SIGNALR_STRING("startChat") },
		{ EASY_STRING("contextKey"), SIGNALR_STRING("UnrealVoxta - SimpleChat") }, // still not sure what this does
		{ EASY_STRING("characterIds"), FSignalRValue(TArray<FSignalRValue> { FSignalRValue(charData->GetId()) }) }
	};

	if (!context.IsEmpty())
	{
		requestData.Add(
		{
			EASY_STRING("contexts"),
			FSignalRValue(TArray<FSignalRValue> {
				FSignalRValue(TMap<FString, FSignalRValue> {
					{ EASY_STRING("text"), context }
				})
			})
		});
	}

	// Req for action inference
	// { EASY_STRING("actions"), ... }

	// Req for scenario support
	// { EASY_STRING("scenarioId"), ... }

	// Req for multi-character support (possibly also scenarios with multiple characters?)
	// { EASY_STRING("roles"), ... }

	// Not sure why starting a chat would require a chat id? maybe to continue? Need to be figured out
	// { EASY_STRING("chatId"), ... }

	// Not sure what chat dependencies are, need to figure it out
	// { EASY_STRING("dependencies"), ... }

	return FSignalRValue(requestData);
}

FSignalRValue VoxtaApiRequestHandler::GetSendUserMessageData(const FString& sessionId,
	const FString& userInputText, bool generateReply, bool characterActionInference) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("send") },
		{ EASY_STRING("sessionId"), FSignalRValue(sessionId) },
		{ EASY_STRING("text"), FSignalRValue(userInputText) },
		{ EASY_STRING("doReply"), generateReply ? SIGNALR_STRING("true") : SIGNALR_STRING("false") },
		{ EASY_STRING("doCharacterActionInference"), characterActionInference ? SIGNALR_STRING("true") : SIGNALR_STRING("false") }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetNotifyAudioPlaybackCompletedData(const FString& sessionId,
	const FString& messageId) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("speechPlaybackComplete") },
		{ EASY_STRING("sessionId"), FSignalRValue(sessionId) },
		{ EASY_STRING("messageId"), FSignalRValue(messageId) }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetNotifyAudioPlaybackStartedData(const FString& sessionId,
	const FString& messageId, int startIndex, int endIndex, double duration) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("speechPlaybackStart") },
		{ EASY_STRING("sessionId"), FSignalRValue(sessionId) },
		{ EASY_STRING("messageId"), FSignalRValue(messageId) },
		{ EASY_STRING("startIndex"), FSignalRValue(startIndex) },
		{ EASY_STRING("endIndex"), FSignalRValue(endIndex) },
		{ EASY_STRING("duration"), FSignalRValue(FString::Printf(TEXT("%.3f"), duration)) }
	});
}