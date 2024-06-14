// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiResponseHandler.h"
#include "VoxtaLogUtility.h"

TUniquePtr<ServerResponseBase> VoxtaApiResponseHandler::GetResponseData(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	using enum ServerResponseType;
	const FString type = serverResponseData[API_STRING("$type")].AsString();
	if (type == "welcome")
	{
		return GetWelcomeResponse(serverResponseData);
	}
	else if (type == "charactersListLoaded")
	{
		return GetCharacterListLoadedResponse(serverResponseData);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Failed to process VoxtaApiResponse of type: {type}.", type);
		return nullptr;
	}
}

TUniquePtr<ServerResponseWelcome> VoxtaApiResponseHandler::GetWelcomeResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	auto& user = serverResponseData[API_STRING("user")].AsObject();
	return MakeUnique<ServerResponseWelcome>(
		FCharData(user[API_STRING("id")].AsString(), user[API_STRING("name")].AsString()));
}

TUniquePtr<ServerResponseCharacterList> VoxtaApiResponseHandler::GetCharacterListLoadedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	auto& charArray = serverResponseData[API_STRING("characters")].AsArray();
	TArray<FCharData> chars;
	chars.Reserve(charArray.Num());
	for (auto& charElement : charArray)
	{
		auto& characterData = charElement.AsObject();
		auto character = FCharData(characterData[API_STRING("id")].AsString(), characterData[API_STRING("name")].AsString());
		character.m_creatorNotes = characterData.Contains(API_STRING("creatorNotes")) ? characterData[API_STRING("creatorNotes")].AsString() : "";
		character.m_explicitContent = characterData.Contains(API_STRING("explicitContent")) ? characterData[API_STRING("explicitContent")].AsBool() : false;
		character.m_favorite = characterData.Contains(API_STRING("favorite")) ? characterData[API_STRING("favorite")].AsBool() : false;
		chars.Emplace(character);
	}
	return MakeUnique<ServerResponseCharacterList>(chars);
}