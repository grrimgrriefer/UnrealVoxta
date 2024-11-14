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
		{ EASY_STRING("scope"), FSignalRValue(TArray<FSignalRValue> {
			SIGNALR_STRING("role:app")/*,
			SIGNALR_STRING("broadcast:write")*/ }) },
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

//FSignalRValue VoxtaApiRequestHandler::GetLoadCharacterRequestData(const FString& characterId) const
//{
//	return FSignalRValue(TMap<FString, FSignalRValue> {
//		{ EASY_STRING("$type"), SIGNALR_STRING("loadCharacter") },
//		{ EASY_STRING("characterId"), FSignalRValue(characterId) }
//	});
//}

FSignalRValue VoxtaApiRequestHandler::GetStartChatRequestData(const FAiCharData* charData) const
{
	auto characterParams = TMap<FString, FSignalRValue>{
		{ EASY_STRING("id"), FSignalRValue(charData->GetId()) },
		{ EASY_STRING("name"), SIGNALR_STRINGVIEW(charData->GetName()) },
		{ EASY_STRING("explicitContent"), SIGNALR_STRING("True")} };

	return FSignalRValue(TMap<FString, FSignalRValue>{
		{ EASY_STRING("$type"), SIGNALR_STRING("startChat") },
		{ EASY_STRING("contextKey"), SIGNALR_STRING("") },
		{ EASY_STRING("context"), SIGNALR_STRING("") },
		{ EASY_STRING("chatId"), FSignalRValue(FGuid::NewGuid().ToString()) },
		{ EASY_STRING("characterId"), FSignalRValue(charData->GetId()) },
		{ EASY_STRING("character"), FSignalRValue(characterParams) }
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