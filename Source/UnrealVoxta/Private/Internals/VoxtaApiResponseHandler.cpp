// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiResponseHandler.h"
#include "VoxtaLogger.h"

TUniquePtr<IServerResponseBase> VoxtaApiResponseHandler::GetResponseData(
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
	else if (type == "replyCancelled")
	{
		return GetReplyCancelledResponse(serverResponseData);
	}
	else if (type == "update")
	{
		return GetChatUpdateResponse(serverResponseData);
	}
	else if (type == "speechRecognitionPartial")
	{
		return GetSpeechRecognitionPartial(serverResponseData);
	}
	else if (type == "speechRecognitionEnd")
	{
		return GetSpeechRecognitionEnd(serverResponseData);
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
		FUserCharData(user[API_STRING("id")].AsString(), user[API_STRING("name")].AsString()));
}

TUniquePtr<ServerResponseCharacterList> VoxtaApiResponseHandler::GetCharacterListLoadedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	auto& charArray = serverResponseData[API_STRING("characters")].AsArray();
	TArray<FAiCharData> chars;
	chars.Reserve(charArray.Num());
	for (auto& charElement : charArray)
	{
		auto& characterData = charElement.AsObject();
		auto character = FAiCharData(characterData[API_STRING("id")].AsString(),
			characterData[API_STRING("name")].AsString(),
			characterData.Contains(API_STRING("creatorNotes")) ? characterData[API_STRING("creatorNotes")].AsString() : "",
			characterData.Contains(API_STRING("explicitContent")) ? characterData[API_STRING("explicitContent")].AsBool() : false,
			characterData.Contains(API_STRING("favorite")) ? characterData[API_STRING("favorite")].AsBool() : false);
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
		{ TextGen, "textGen" },
		{ SpeechToText, "speechToText" },
		{ TextToSpeech, "textToSpeech" }
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

TUniquePtr<ServerResponseChatMessageStart> VoxtaApiResponseHandler::GetReplyStartReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessageStart>(
		serverResponseData[API_STRING("messageId")].AsString(),
		serverResponseData[API_STRING("senderId")].AsString(),
		serverResponseData[API_STRING("sessionId")].AsString());
}

TUniquePtr<ServerResponseChatMessageChunk> VoxtaApiResponseHandler::GetReplyChunkReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessageChunk>(
		serverResponseData[API_STRING("messageId")].AsString(),
		serverResponseData[API_STRING("senderId")].AsString(),
		serverResponseData[API_STRING("sessionId")].AsString(),
		static_cast<int>(serverResponseData[API_STRING("startIndex")].AsDouble()),
		static_cast<int>(serverResponseData[API_STRING("endIndex")].AsDouble()),
		serverResponseData[API_STRING("text")].AsString(),
		serverResponseData[API_STRING("audioUrl")].AsString());
}

TUniquePtr<ServerResponseChatMessageEnd> VoxtaApiResponseHandler::GetReplyEndReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessageEnd>(
		serverResponseData[API_STRING("messageId")].AsString(),
		serverResponseData[API_STRING("senderId")].AsString(),
		serverResponseData[API_STRING("sessionId")].AsString());
}

TUniquePtr<ServerResponseChatMessageCancelled> VoxtaApiResponseHandler::GetReplyCancelledResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessageCancelled>(
		serverResponseData[API_STRING("messageId")].AsString(),
		serverResponseData[API_STRING("sessionId")].AsString());
}

TUniquePtr<ServerResponseChatUpdate> VoxtaApiResponseHandler::GetChatUpdateResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatUpdate>(
		serverResponseData[API_STRING("messageId")].AsString(),
		serverResponseData[API_STRING("senderId")].AsString(),
		serverResponseData[API_STRING("text")].AsString(),
		serverResponseData[API_STRING("sessionId")].AsString());
}

TUniquePtr<ServerResponseSpeechTranscription> VoxtaApiResponseHandler::GetSpeechRecognitionPartial(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseSpeechTranscription>(
		serverResponseData[API_STRING("text")].AsString(), ServerResponseSpeechTranscription::TranscriptionState::PARTIAL);
}

TUniquePtr<ServerResponseSpeechTranscription> VoxtaApiResponseHandler::GetSpeechRecognitionEnd(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	bool isValid = serverResponseData.Contains(API_STRING("text"));
	return MakeUnique<ServerResponseSpeechTranscription>(
		isValid ? serverResponseData[API_STRING("text")].AsString() : FString(),
		isValid ? ServerResponseSpeechTranscription::TranscriptionState::END
		: ServerResponseSpeechTranscription::TranscriptionState::CANCELLED);
}