// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiResponseHandler.h"
#include "Logging/StructuredLog.h"
#include "SignalR/Public/SignalRValue.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseBase.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseWelcome.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseCharacterList.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatStarted.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatMessageStart.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatMessageChunk.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatMessageEnd.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatMessageCancelled.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatUpdate.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseSpeechTranscription.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseError.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseContextUpdated.h"
#include "VoxtaData/Public/ServerResponses/ServerResponseChatClosed.h"
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
	else if (type == TEXT("error"))
	{
		return GetErrorResponse(serverResponseData);
	}
	else if (type == TEXT("contextUpdated"))
	{
		return GetContextUpdatedResponse(serverResponseData);
	}
	else if (type == TEXT("chatClosed"))
	{
		return GetChatClosedResponse(serverResponseData);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Failed to process VoxtaApiResponse of type: {0}.", type);
		return nullptr;
	}
}

TUniquePtr<ServerResponseWelcome> VoxtaApiResponseHandler::GetWelcomeResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	TMap<FString, FSignalRValue> user = serverResponseData[EASY_STRING("user")].AsObject();
	//FString voxtaServerVersion = serverResponseData[EASY_STRING("voxtaServerVersion")].AsString();
	//FString apiVersion = serverResponseData[EASY_STRING("apiVersion")].AsString();
	return MakeUnique<ServerResponseWelcome>(FUserCharData(GetStringAsGuid(user[EASY_STRING("id")]),
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
			GetStringAsGuid(characterData[EASY_STRING("id")]),
			characterData[EASY_STRING("name")].AsString(),
			characterData.Contains(EASY_STRING("creatorNotes"))
			? characterData[EASY_STRING("creatorNotes")].AsString() : EMPTY_STRING,
			characterData.Contains(EASY_STRING("explicitContent"))
			? characterData[EASY_STRING("explicitContent")].AsBool() : false,
			characterData.Contains(EASY_STRING("favorite"))
			? characterData[EASY_STRING("favorite")].AsBool() : false,
			characterData.Contains(EASY_STRING("thumbnailUrl"))
			? characterData[EASY_STRING("thumbnailUrl")].AsString() : EMPTY_STRING,
			characterData.Contains(EASY_STRING("packageId"))
			? GetStringAsGuid(characterData[EASY_STRING("packageId")]) : FGuid(),
			characterData.Contains(EASY_STRING("packageName"))
			? characterData[EASY_STRING("packageName")].AsString() : EMPTY_STRING));
	}
	return MakeUnique<ServerResponseCharacterList>(chars);
}

TUniquePtr<ServerResponseContextUpdated> VoxtaApiResponseHandler::GetContextUpdatedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	FString contextValue = FString();
	ProcessContextData(serverResponseData, contextValue);

	return MakeUnique<ServerResponseContextUpdated>(
		contextValue,
		GetStringAsGuid(serverResponseData[EASY_STRING("sessionId")]));
}

TUniquePtr<ServerResponseChatStarted> VoxtaApiResponseHandler::GetChatStartedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	// Participants
	TMap<FString, FSignalRValue> user = serverResponseData[EASY_STRING("user")].AsObject();
	TArray<FSignalRValue> charIdArray = serverResponseData[EASY_STRING("characters")].AsArray();
	TArray<FGuid> chars;
	chars.Reserve(charIdArray.Num());
	for (const FSignalRValue& charElement : charIdArray)
	{
		const TMap<FString, FSignalRValue>& characterData = charElement.AsObject();
		chars.Emplace(GetStringAsGuid(characterData[EASY_STRING("id")]));
	}

	// Services
	TMap<FString, FSignalRValue> servicesMap = serverResponseData[EASY_STRING("services")].AsObject();
	using enum VoxtaServiceType;
	TMap<VoxtaServiceType, FVoxtaServiceData> services;
	TMap<VoxtaServiceType, FString> serviceTypes = {
		{ TextGen, TEXT("textGen") },
		{ SpeechToText, TEXT("speechToText") },
		{ TextToSpeech, TEXT("textToSpeech") }
	};
	for (const auto& [enumType, stringValue] : serviceTypes)
	{
		if (servicesMap.Contains(stringValue))
		{
			const TMap<FString, FSignalRValue>& serviceData = servicesMap[stringValue].AsObject();
			services.Emplace(enumType, FVoxtaServiceData(enumType, serviceData[EASY_STRING("serviceName")].AsString(),
				GetStringAsGuid(serviceData[EASY_STRING("serviceId")])));
		}
	}

	// Context
	TMap<FString, FSignalRValue> context = serverResponseData[EASY_STRING("context")].AsObject();
	FString contextValue = FString();
	ProcessContextData(context, contextValue);

	return MakeUnique<ServerResponseChatStarted>(GetStringAsGuid(user[EASY_STRING("id")]),
		chars, services, GetStringAsGuid(serverResponseData[EASY_STRING("chatId")]),
		GetStringAsGuid(serverResponseData[EASY_STRING("sessionId")]),
		contextValue);
}

TUniquePtr<ServerResponseChatMessageStart> VoxtaApiResponseHandler::GetReplyStartReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessageStart>(
		GetStringAsGuid(serverResponseData[EASY_STRING("messageId")]),
		GetStringAsGuid(serverResponseData[EASY_STRING("senderId")]),
		GetStringAsGuid(serverResponseData[EASY_STRING("sessionId")]));
}

TUniquePtr<ServerResponseChatMessageChunk> VoxtaApiResponseHandler::GetReplyChunkReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessageChunk>(
		GetStringAsGuid(serverResponseData[EASY_STRING("messageId")]),
		GetStringAsGuid(serverResponseData[EASY_STRING("senderId")]),
		GetStringAsGuid(serverResponseData[EASY_STRING("sessionId")]),
		static_cast<int>(serverResponseData[EASY_STRING("startIndex")].AsDouble()),
		static_cast<int>(serverResponseData[EASY_STRING("endIndex")].AsDouble()),
		serverResponseData[EASY_STRING("text")].AsString(),
		serverResponseData[EASY_STRING("audioUrl")].AsString(),
		serverResponseData[EASY_STRING("isNarration")].AsBool());
}

TUniquePtr<ServerResponseChatMessageEnd> VoxtaApiResponseHandler::GetReplyEndReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessageEnd>(
		GetStringAsGuid(serverResponseData[EASY_STRING("messageId")]),
		GetStringAsGuid(serverResponseData[EASY_STRING("senderId")]),
		GetStringAsGuid(serverResponseData[EASY_STRING("sessionId")]));
}

TUniquePtr<ServerResponseChatMessageCancelled> VoxtaApiResponseHandler::GetReplyCancelledResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatMessageCancelled>(
		GetStringAsGuid(serverResponseData[EASY_STRING("messageId")]),
		GetStringAsGuid(serverResponseData[EASY_STRING("sessionId")]));
}

TUniquePtr<ServerResponseChatUpdate> VoxtaApiResponseHandler::GetChatUpdateResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatUpdate>(
		GetStringAsGuid(serverResponseData[EASY_STRING("messageId")]),
		GetStringAsGuid(serverResponseData[EASY_STRING("senderId")]),
		serverResponseData[EASY_STRING("text")].AsString(),
		GetStringAsGuid(serverResponseData[EASY_STRING("sessionId")]));
}

TUniquePtr<ServerResponseChatClosed> VoxtaApiResponseHandler::GetChatClosedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseChatClosed>(
		GetStringAsGuid(serverResponseData[EASY_STRING("chatId")]),
		GetStringAsGuid(serverResponseData[EASY_STRING("sessionId")]));
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

TUniquePtr<ServerResponseError> VoxtaApiResponseHandler::GetErrorResponse(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	return MakeUnique<ServerResponseError>(
		serverResponseData[EASY_STRING("message")].AsString(),
		serverResponseData[EASY_STRING("details")].AsString());
}

void VoxtaApiResponseHandler::ProcessContextData(TMap<FString, FSignalRValue> contextMainObject,
	FString& outContextValue) const
{
	//TArray<FSignalRValue> flagsArray = serverResponseData[EASY_STRING("flags")].AsArray();
	TArray<FSignalRValue> contextsArray = contextMainObject[EASY_STRING("contexts")].AsArray();
	//TArray<FSignalRValue> actionsArray = serverResponseData[EASY_STRING("actions")].AsArray();
	//TArray<FSignalRValue> charactersArray = serverResponseData[EASY_STRING("characters")].AsArray();
	//TArray<FSignalRValue> rolesArray = serverResponseData[EASY_STRING("roles")].AsArray();

	outContextValue = FString();

	for (const FSignalRValue& context : contextsArray)
	{
		const TMap<FString, FSignalRValue>& contextData = context.AsObject();
		if (contextData[EASY_STRING("contextKey")].AsString() == VOXTA_CONTEXT_KEY)
		{
			outContextValue = contextData[EASY_STRING("text")].AsString();
		}
	}
}

FGuid VoxtaApiResponseHandler::GetStringAsGuid(const FSignalRValue& input) const
{
	return GetStringAsGuid(input.AsString());
}

FGuid VoxtaApiResponseHandler::GetStringAsGuid(const FString& input) const
{
	FGuid userID;
	bool success = FGuid::Parse(input, userID);
	return userID;
}