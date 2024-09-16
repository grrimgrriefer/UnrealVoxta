// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiRequestHandler.h"
#include "VoxtaDefines.h"
#include "SignalR/Public/SignalRValue.h"
#include "AiCharData.h"

FSignalRValue VoxtaApiRequestHandler::GetAuthenticateRequestData() const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ API_STRING("$type"), API_SIGNALR_STRING("authenticate") },
		{ API_STRING("client"), API_SIGNALR_STRING("UnrealVoxta") },
		{ API_STRING("clientVersion"), API_SIGNALR_STRING("0.0.1-preAlpha") },
		{ API_STRING("scope"), FSignalRValue(TArray<FSignalRValue> {
			API_SIGNALR_STRING("role:app"),
				API_SIGNALR_STRING("broadcast:write") }) },
		{ API_STRING("capabilities"), FSignalRValue(TMap<FString, FSignalRValue> {
			{ API_STRING("audioInput"),  API_SIGNALR_STRING("WebSocketStream") },
			{ API_STRING("audioOutput"),  API_SIGNALR_STRING("Url") },
			{ API_STRING("acceptedAudioContentTypes"), FSignalRValue(TArray<FSignalRValue> {
				API_SIGNALR_STRING("audio/x-wav") }) }
		}) }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetLoadCharactersListData() const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ API_STRING("$type"), API_SIGNALR_STRING("loadCharactersList") }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetLoadCharacterRequestData(const FString& characterId) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ API_STRING("$type"), API_SIGNALR_STRING("loadCharacter") },
		{ API_STRING("characterId"), FSignalRValue(characterId) }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetStartChatRequestData(const FAiCharData* charData) const
{
	FString explicitContent = charData->GetIsAllowedExplicitContent() ? "True" : "False";

	auto characterParams = TMap<FString, FSignalRValue>{
		{ API_STRING("id"), API_SIGNALR_STRINGVIEW(charData->GetId()) },
		{ API_STRING("name"), API_SIGNALR_STRINGVIEW(charData->GetName()) },
		{ API_STRING("explicitContent"), FSignalRValue(explicitContent) } };

	return FSignalRValue(TMap<FString, FSignalRValue>{
		{ API_STRING("$type"), API_SIGNALR_STRING("startChat") },
		{ API_STRING("contextKey"), API_SIGNALR_STRING("") },
		{ API_STRING("context"), API_SIGNALR_STRING("") },
		{ API_STRING("chatId"), FSignalRValue(FGuid::NewGuid().ToString()) },
		{ API_STRING("characterId"), API_SIGNALR_STRINGVIEW(charData->GetId()) },
		{ API_STRING("character"), FSignalRValue(characterParams) }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetSendUserMessageData(const FString& sessionId,
	const FString& userInputText) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ API_STRING("$type"), API_SIGNALR_STRING("send") },
		{ API_STRING("sessionId"), FSignalRValue(sessionId) },
		{ API_STRING("text"), FSignalRValue(userInputText) },
		{ API_STRING("doReply"), API_SIGNALR_STRING("true") },
		{ API_STRING("doCharacterActionInference"), API_SIGNALR_STRING("false") }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetNotifyAudioPlaybackCompleteData(const FString& sessionId,
	const FString& messageId) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ API_STRING("$type"), API_SIGNALR_STRING("speechPlaybackComplete") },
		{ API_STRING("sessionId"), FSignalRValue(sessionId) },
		{ API_STRING("messageId"), FSignalRValue(messageId) }
	});
}