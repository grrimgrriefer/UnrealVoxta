// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiRequestHandler.h"
#include "VoxtaDefines.h"

FSignalRValue VoxtaApiRequestHandler::GetAuthenticateRequestData() const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ API_STRING("$type"), API_SIGNALR_STRING("authenticate") },
		{ API_STRING("client"), API_SIGNALR_STRING("TalkToMeCppUE") },
		{ API_STRING("clientVersion"), API_SIGNALR_STRING("0.0.1a") },
		{ API_STRING("scope"), FSignalRValue(TArray<FSignalRValue> { API_SIGNALR_STRING("role:app"), API_SIGNALR_STRING("broadcast:write") }) },
		{ API_STRING("capabilities"), FSignalRValue(TMap<FString, FSignalRValue> {
			{ API_STRING("audioInput"),  API_SIGNALR_STRING("WebSocketStream") },
			{ API_STRING("audioOutput"),  API_SIGNALR_STRING("Url") },
			{ API_STRING("acceptedAudioContentTypes"), FSignalRValue(TArray<FSignalRValue> { API_SIGNALR_STRING("audio/x-wav") }) }
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

FSignalRValue VoxtaApiRequestHandler::GetStartChatRequestData(const FCharData* charData) const
{
	FString explicitContent = charData->m_explicitContent ? "True" : "False";

	auto characterParams = TMap<FString, FSignalRValue>{
		{ API_STRING("id"), FSignalRValue(charData->m_id) },
		{ API_STRING("name"), FSignalRValue(charData->m_name) },
		{ API_STRING("explicitContent"), FSignalRValue(explicitContent) } };

	return FSignalRValue(TMap<FString, FSignalRValue>{
		{ API_STRING("$type"), API_SIGNALR_STRING("startChat") },
		{ API_STRING("contextKey"), API_SIGNALR_STRING("") },
		{ API_STRING("context"), API_SIGNALR_STRING("") },
		{ API_STRING("chatId"), FSignalRValue(FGuid::NewGuid().ToString()) },
		{ API_STRING("characterId"), FSignalRValue(charData->m_id) },
		{ API_STRING("character"), FSignalRValue(characterParams) }
	});
}