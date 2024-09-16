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

TUniquePtr<IServerResponseBase> VoxtaApiResponseHandler::GetResponseData(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	using enum ServerResponseType;
	const FString type = serverResponseData[API_STRING("$type")].AsString();
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
	TMap<FString, FSignalRValue> user = serverResponseData[API_STRING("user")].AsObject();
	return MakeUnique<ServerResponseWelcome>(FUserCharData(user[API_STRING("id")].AsString(),
		user[API_STRING("name")].AsString()));
}

TUniquePtr<ServerResponseCharacterList> VoxtaApiResponseHandler::GetCharacterListLoadedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	TArray<FSignalRValue> charArray = serverResponseData[API_STRING("characters")].AsArray();
	TArray<FAiCharData> chars;
	chars.Reserve(charArray.Num());
	for (const FSignalRValue& charElement : charArray)
	{
		const TMap<FString, FSignalRValue>& characterData = charElement.AsObject();
		chars.Emplace(FAiCharData(
			characterData[API_STRING("id")].AsString(),
			characterData[API_STRING("name")].AsString(),
			characterData.Contains(API_STRING("creatorNotes"))
			? characterData[API_STRING("creatorNotes")].AsString() : "",
			characterData.Contains(API_STRING("explicitContent"))
			? characterData[API_STRING("explicitContent")].AsBool() : false,
			characterData.Contains(API_STRING("favorite"))
			? characterData[API_STRING("favorite")].AsBool() : false));
	}
	return MakeUnique<ServerResponseCharacterList>(chars);
}

TUniquePtr<ServerResponseCharacterLoaded> VoxtaApiResponseHandler::GetCharacterLoadedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	TMap<FString, FSignalRValue> characterData = serverResponseData[API_STRING("character")].AsObject();
	const TArray<FSignalRValue>& ttsConfig = characterData[API_STRING("textToSpeech")].AsArray();

	return MakeUnique<ServerResponseCharacterLoaded>(characterData[API_STRING("id")].AsString(),
		characterData[API_STRING("enableThinkingSpeech")].AsBool());
}

TUniquePtr<ServerResponseChatStarted> VoxtaApiResponseHandler::GetChatStartedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	TMap<FString, FSignalRValue> user = serverResponseData[API_STRING("user")].AsObject();
	TArray<FSignalRValue> charIdArray = serverResponseData[API_STRING("characters")].AsArray();
	TArray<FString> chars;
	chars.Reserve(charIdArray.Num());
	for (const FSignalRValue& charElement : charIdArray)
	{
		const TMap<FString, FSignalRValue>& characterData = charElement.AsObject();
		chars.Emplace(characterData[API_STRING("id")].AsString());
	}

	TMap<FString, FSignalRValue> servicesMap = serverResponseData[API_STRING("services")].AsObject();
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
			services.Emplace(enumType, VoxtaServiceData(enumType, serviceData[API_STRING("serviceName")].AsString(),
				serviceData[API_STRING("serviceId")].AsString()));
		}
	}
	return MakeUnique<ServerResponseChatStarted>(user[API_STRING("id")].AsString(),
		chars, services, serverResponseData[API_STRING("chatId")].AsString(),
		serverResponseData[API_STRING("sessionId")].AsString());
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
		serverResponseData[API_STRING("text")].AsString(),
		ServerResponseSpeechTranscription::TranscriptionState::PARTIAL);
}

TUniquePtr<ServerResponseSpeechTranscription> VoxtaApiResponseHandler::GetSpeechRecognitionEnd(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	bool isValid = serverResponseData.Contains(API_STRING("text"));
	return MakeUnique<ServerResponseSpeechTranscription>(
		isValid ? serverResponseData[API_STRING("text")].AsString()
			: FString(),
		isValid ? ServerResponseSpeechTranscription::TranscriptionState::END
			: ServerResponseSpeechTranscription::TranscriptionState::CANCELLED);
}