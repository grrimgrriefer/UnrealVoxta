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

FSignalRValue VoxtaApiRequestHandler::GetStartChatRequestData(const FAiCharData* charData) const
{
	return FSignalRValue(TMap<FString, FSignalRValue>{
		{ EASY_STRING("$type"), SIGNALR_STRING("startChat") },
		{ EASY_STRING("contextKey"), SIGNALR_STRING("") },
		{ EASY_STRING("characterIds"), FSignalRValue(TArray<FSignalRValue> { FSignalRValue(charData->GetId()) }) }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetSendUserMessageData(const FString& sessionId,
	const FString& userInputText) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("send") },
		{ EASY_STRING("sessionId"), FSignalRValue(sessionId) },
		{ EASY_STRING("text"), FSignalRValue(userInputText) },
		{ EASY_STRING("doReply"), SIGNALR_STRING("true") },
		{ EASY_STRING("doCharacterActionInference"), SIGNALR_STRING("false") }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetNotifyAudioPlaybackCompleteData(const FString& sessionId,
	const FString& messageId) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("speechPlaybackComplete") },
		{ EASY_STRING("sessionId"), FSignalRValue(sessionId) },
		{ EASY_STRING("messageId"), FSignalRValue(messageId) }
	});
}