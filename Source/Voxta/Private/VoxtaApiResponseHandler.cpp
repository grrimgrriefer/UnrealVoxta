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
	else if (type == "characterLoaded")
	{
		return GetCharacterLoadedResponse(serverResponseData);
	}
	else if (type == "chatStarted")
	{
		return GetChatStartedResponse(serverResponseData);
	}
	else if (type == "replyStart")
	{
		return GetReplyStartReponseResponse(serverResponseData);
	}
	else if (type == "replyChunk")
	{
		return GetReplyChunkReponseResponse(serverResponseData);
	}
	else if (type == "replyEnd")
	{
		return GetReplyEndReponseResponse(serverResponseData);
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

TUniquePtr<ServerResponseCharacterLoaded> VoxtaApiResponseHandler::GetCharacterLoadedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	auto& characterData = serverResponseData[API_STRING("character")].AsObject();
	auto& ttsConfig = characterData[API_STRING("textToSpeech")].AsArray();

	return MakeUnique<ServerResponseCharacterLoaded>(characterData[API_STRING("id")].AsString(),
		characterData[API_STRING("enableThinkingSpeech")].AsBool());
}

TUniquePtr<ServerResponseChatStarted> VoxtaApiResponseHandler::GetChatStartedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	auto& user = serverResponseData[API_STRING("user")].AsObject();
	auto& charIdArray = serverResponseData[API_STRING("characters")].AsArray();
	TArray<FString> chars;
	chars.Reserve(charIdArray.Num());
	for (auto& charElement : charIdArray)
	{
		auto& characterData = charElement.AsObject();
		chars.Emplace(characterData[API_STRING("id")].AsString());
	}

	auto& servicesMap = serverResponseData[API_STRING("services")].AsObject();
	using enum VoxtaServiceData::ServiceType;
	TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData> services;
	TMap<VoxtaServiceData::ServiceType, FString> serviceTypes = {
		{ TEXT_GEN, "textGen" },
		{ SPEECH_TO_TEXT, "speechToText" },
		{ TEXT_TO_SPEECH, "textToSpeech" }
	};

	for (const auto& [enumType, stringValue] : serviceTypes)
	{
		if (servicesMap.Contains(stringValue))
		{
			auto& serviceData = servicesMap[stringValue].AsObject();
			services.Emplace(enumType, VoxtaServiceData(enumType, serviceData[API_STRING("serviceName")].AsString(),
				serviceData[API_STRING("serviceId")].AsString()));
		}
	}
	return MakeUnique<ServerResponseChatStarted>(user[API_STRING("id")].AsString(),
		chars, services, serverResponseData[API_STRING("chatId")].AsString(), serverResponseData[API_STRING("sessionId")].AsString());
}

TUniquePtr<ServerResponseChatMessage> VoxtaApiResponseHandler::GetReplyStartReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessage>(
		ServerResponseChatMessage::MessageType::MESSAGE_START,
		serverResponseData[API_STRING("messageId")].AsString(),
		serverResponseData[API_STRING("senderId")].AsString(),
		serverResponseData[API_STRING("sessionId")].AsString());
}

TUniquePtr<ServerResponseChatMessage> VoxtaApiResponseHandler::GetReplyChunkReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessage>(
		ServerResponseChatMessage::MessageType::MESSAGE_CHUNK,
		serverResponseData[API_STRING("messageId")].AsString(),
		serverResponseData[API_STRING("senderId")].AsString(),
		serverResponseData[API_STRING("sessionId")].AsString(),
		static_cast<int>(serverResponseData[API_STRING("startIndex")].AsDouble()),
		static_cast<int>(serverResponseData[API_STRING("endIndex")].AsDouble()),
		serverResponseData[API_STRING("sessionId")].AsString(),
		serverResponseData[API_STRING("sessionId")].AsString());
}

TUniquePtr<ServerResponseChatMessage> VoxtaApiResponseHandler::GetReplyEndReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessage>(
		ServerResponseChatMessage::MessageType::MESSAGE_END,
		serverResponseData[API_STRING("messageId")].AsString(),
		serverResponseData[API_STRING("senderId")].AsString(),
		serverResponseData[API_STRING("sessionId")].AsString());
}