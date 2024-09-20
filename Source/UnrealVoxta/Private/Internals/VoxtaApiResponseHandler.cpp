// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiResponseHandler.h"
#include "Logging/StructuredLog.h"
#include "SignalR/Public/SignalRValue.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseBase.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseWelcome.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseCharacterList.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseCharacterLoaded.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatStarted.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatMessageStart.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatMessageChunk.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatMessageEnd.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatMessageCancelled.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatUpdate.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseSpeechTranscription.h"
#include "VoxtaData/Public/VoxtaServiceData.h"

TUniquePtr<ServerResponseBase> VoxtaApiResponseHandler::GetResponseData(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	using enum ServerResponseType;
	const FString type = serverResponseData[EASY_STRING("$type")].AsString();
	if (type == TEXT("welcome"))
	{
		return GetWelcomeResponse(serverResponseData);
	}
	else if (type == TEXT("charactersListLoaded"))
	{
		return GetCharacterListLoadedResponse(serverResponseData);
	}
	else if (type == TEXT("characterLoaded"))
	{
		return GetCharacterLoadedResponse(serverResponseData);
	}
	else if (type == TEXT("chatStarted"))
	{
		return GetChatStartedResponse(serverResponseData);
	}
	else if (type == TEXT("replyStart"))
	{
		return GetReplyStartReponseResponse(serverResponseData);
	}
	else if (type == TEXT("replyChunk"))
	{
		return GetReplyChunkReponseResponse(serverResponseData);
	}
	else if (type == TEXT("replyEnd"))
	{
		return GetReplyEndReponseResponse(serverResponseData);
	}
	else if (type == TEXT("replyCancelled"))
	{
		return GetReplyCancelledResponse(serverResponseData);
	}
	else if (type == TEXT("update"))
	{
		return GetChatUpdateResponse(serverResponseData);
	}
	else if (type == TEXT("speechRecognitionPartial"))
	{
		return GetSpeechRecognitionPartial(serverResponseData);
	}
	else if (type == TEXT("speechRecognitionEnd"))
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
	TMap<FString, FSignalRValue> user = serverResponseData[EASY_STRING("user")].AsObject();
	return MakeUnique<ServerResponseWelcome>(FUserCharData(user[EASY_STRING("id")].AsString(),
		user[EASY_STRING("name")].AsString()));
}

TUniquePtr<ServerResponseCharacterList> VoxtaApiResponseHandler::GetCharacterListLoadedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	TArray<FSignalRValue> charArray = serverResponseData[EASY_STRING("characters")].AsArray();
	TArray<FAiCharData> chars;
	chars.Reserve(charArray.Num());
	for (const FSignalRValue& charElement : charArray)
	{
		const TMap<FString, FSignalRValue>& characterData = charElement.AsObject();
		chars.Emplace(FAiCharData(
			characterData[EASY_STRING("id")].AsString(),
			characterData[EASY_STRING("name")].AsString(),
			characterData.Contains(EASY_STRING("creatorNotes"))
			? characterData[EASY_STRING("creatorNotes")].AsString() : "",
			characterData.Contains(EASY_STRING("explicitContent"))
			? characterData[EASY_STRING("explicitContent")].AsBool() : false,
			characterData.Contains(EASY_STRING("favorite"))
			? characterData[EASY_STRING("favorite")].AsBool() : false));
	}
	return MakeUnique<ServerResponseCharacterList>(chars);
}

TUniquePtr<ServerResponseCharacterLoaded> VoxtaApiResponseHandler::GetCharacterLoadedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	TMap<FString, FSignalRValue> characterData = serverResponseData[EASY_STRING("character")].AsObject();
	const TArray<FSignalRValue>& ttsConfig = characterData[EASY_STRING("textToSpeech")].AsArray();

	return MakeUnique<ServerResponseCharacterLoaded>(characterData[EASY_STRING("id")].AsString(),
		characterData[EASY_STRING("enableThinkingSpeech")].AsBool());
}

TUniquePtr<ServerResponseChatStarted> VoxtaApiResponseHandler::GetChatStartedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	TMap<FString, FSignalRValue> user = serverResponseData[EASY_STRING("user")].AsObject();
	TArray<FSignalRValue> charIdArray = serverResponseData[EASY_STRING("characters")].AsArray();
	TArray<FString> chars;
	chars.Reserve(charIdArray.Num());
	for (const FSignalRValue& charElement : charIdArray)
	{
		const TMap<FString, FSignalRValue>& characterData = charElement.AsObject();
		chars.Emplace(characterData[EASY_STRING("id")].AsString());
	}

	TMap<FString, FSignalRValue> servicesMap = serverResponseData[EASY_STRING("services")].AsObject();
	using enum VoxtaServiceData::ServiceType;
	TMap<const VoxtaServiceData::ServiceType, const VoxtaServiceData> services;
	TMap<VoxtaServiceData::ServiceType, FString> serviceTypes = {
		{ TextGen, TEXT("textGen") },
		{ SpeechToText, TEXT("speechToText") },
		{ TextToSpeech, TEXT("textToSpeech") }
	};

	for (const auto& [enumType, stringValue] : serviceTypes)
	{
		if (servicesMap.Contains(stringValue))
		{
			const TMap<FString, FSignalRValue>& serviceData = servicesMap[stringValue].AsObject();
			services.Emplace(enumType, VoxtaServiceData(enumType, serviceData[EASY_STRING("serviceName")].AsString(),
				serviceData[EASY_STRING("serviceId")].AsString()));
		}
	}
	return MakeUnique<ServerResponseChatStarted>(user[EASY_STRING("id")].AsString(),
		chars, services, serverResponseData[EASY_STRING("chatId")].AsString(),
		serverResponseData[EASY_STRING("sessionId")].AsString());
}

TUniquePtr<ServerResponseChatMessageStart> VoxtaApiResponseHandler::GetReplyStartReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessageStart>(
		serverResponseData[EASY_STRING("messageId")].AsString(),
		serverResponseData[EASY_STRING("senderId")].AsString(),
		serverResponseData[EASY_STRING("sessionId")].AsString());
}

TUniquePtr<ServerResponseChatMessageChunk> VoxtaApiResponseHandler::GetReplyChunkReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessageChunk>(
		serverResponseData[EASY_STRING("messageId")].AsString(),
		serverResponseData[EASY_STRING("senderId")].AsString(),
		serverResponseData[EASY_STRING("sessionId")].AsString(),
		static_cast<int>(serverResponseData[EASY_STRING("startIndex")].AsDouble()),
		static_cast<int>(serverResponseData[EASY_STRING("endIndex")].AsDouble()),
		serverResponseData[EASY_STRING("text")].AsString(),
		serverResponseData[EASY_STRING("audioUrl")].AsString());
}

TUniquePtr<ServerResponseChatMessageEnd> VoxtaApiResponseHandler::GetReplyEndReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessageEnd>(
		serverResponseData[EASY_STRING("messageId")].AsString(),
		serverResponseData[EASY_STRING("senderId")].AsString(),
		serverResponseData[EASY_STRING("sessionId")].AsString());
}

TUniquePtr<ServerResponseChatMessageCancelled> VoxtaApiResponseHandler::GetReplyCancelledResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessageCancelled>(
		serverResponseData[EASY_STRING("messageId")].AsString(),
		serverResponseData[EASY_STRING("sessionId")].AsString());
}

TUniquePtr<ServerResponseChatUpdate> VoxtaApiResponseHandler::GetChatUpdateResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatUpdate>(
		serverResponseData[EASY_STRING("messageId")].AsString(),
		serverResponseData[EASY_STRING("senderId")].AsString(),
		serverResponseData[EASY_STRING("text")].AsString(),
		serverResponseData[EASY_STRING("sessionId")].AsString());
}

TUniquePtr<ServerResponseSpeechTranscription> VoxtaApiResponseHandler::GetSpeechRecognitionPartial(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseSpeechTranscription>(
		serverResponseData[EASY_STRING("text")].AsString(),
		ServerResponseSpeechTranscription::TranscriptionState::PARTIAL);
}

TUniquePtr<ServerResponseSpeechTranscription> VoxtaApiResponseHandler::GetSpeechRecognitionEnd(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	bool isValid = serverResponseData.Contains(EASY_STRING("text"));
	return MakeUnique<ServerResponseSpeechTranscription>(
		isValid ? serverResponseData[EASY_STRING("text")].AsString()
			: FString(),
		isValid ? ServerResponseSpeechTranscription::TranscriptionState::END
			: ServerResponseSpeechTranscription::TranscriptionState::CANCELLED);
}