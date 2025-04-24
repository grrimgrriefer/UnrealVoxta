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

#define SAFE_MAP_GET(Map, Key)                                             \
    ([&]() -> const FSignalRValue&                                         \
    {                                                                      \
        const auto& _key = (Key);                                          \
        if ((Map).Contains(_key))                                          \
        {                                                                  \
            return (Map)[_key];                                            \
        }                                                                  \
        UE_LOGFMT(VoxtaLog, Error, "Map missing key: {0}", *(_key));       \
        static const FSignalRValue _empty;                                 \
        return _empty;                                                     \
    }())

const TSet<FString> VoxtaApiResponseHandler::IGNORED_MESSAGE_TYPES{
		EASY_STRING("chatStarting"),
		EASY_STRING("chatLoadingMessage"),
		EASY_STRING("chatsSessionsUpdated"),
		EASY_STRING("replyGenerating"),
		EASY_STRING("chatFlow"),
		EASY_STRING("speechRecognitionStart"),
		EASY_STRING("recordingRequest"),
		EASY_STRING("recordingStatus"),
		EASY_STRING("speechPlaybackComplete"),
		EASY_STRING("memoryUpdated")
};

TUniquePtr<ServerResponseBase> VoxtaApiResponseHandler::GetResponseData(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	using enum ServerResponseType;
	const FString type = SAFE_MAP_GET(serverResponseData, EASY_STRING("$type")).AsString();
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
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	TMap<FString, FSignalRValue> user = SAFE_MAP_GET(serverResponseData, EASY_STRING("user")).AsObject();
	TMap<FString, FSignalRValue> assistant = SAFE_MAP_GET(serverResponseData, EASY_STRING("assistant")).AsObject();
	FString voxtaServerVersion = SAFE_MAP_GET(serverResponseData, EASY_STRING("voxtaServerVersion")).AsString();
	FString apiVersion = SAFE_MAP_GET(serverResponseData, EASY_STRING("apiVersion")).AsString();
	return MakeUnique<ServerResponseWelcome>(FUserCharData(GetStringAsGuid(SAFE_MAP_GET(user, EASY_STRING("id"))),
		SAFE_MAP_GET(user, EASY_STRING("name")).AsString()), GetStringAsGuid(SAFE_MAP_GET(assistant, EASY_STRING("id"))),
		voxtaServerVersion, apiVersion);
}

TUniquePtr<ServerResponseCharacterList> VoxtaApiResponseHandler::GetCharacterListLoadedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	TArray<FSignalRValue> charArray = SAFE_MAP_GET(serverResponseData, EASY_STRING("characters")).AsArray();
	TArray<FAiCharData> chars;
	chars.Reserve(charArray.Num());
	for (const FSignalRValue& charElement : charArray)
	{
		const TMap<FString, FSignalRValue>& characterData = charElement.AsObject();
		chars.Emplace(FAiCharData(
			GetStringAsGuid(SAFE_MAP_GET(characterData, EASY_STRING("id"))),
			SAFE_MAP_GET(characterData, EASY_STRING("name")).AsString(),
			characterData.Contains(EASY_STRING("creatorNotes"))
			? SAFE_MAP_GET(characterData, EASY_STRING("creatorNotes")).AsString() : EMPTY_STRING,
			characterData.Contains(EASY_STRING("explicitContent"))
			? SAFE_MAP_GET(characterData, EASY_STRING("explicitContent")).AsBool() : false,
			characterData.Contains(EASY_STRING("favorite"))
			? SAFE_MAP_GET(characterData, EASY_STRING("favorite")).AsBool() : false,
			characterData.Contains(EASY_STRING("thumbnailUrl"))
			? SAFE_MAP_GET(characterData, EASY_STRING("thumbnailUrl")).AsString() : EMPTY_STRING,
			characterData.Contains(EASY_STRING("packageId"))
			? GetStringAsGuid(SAFE_MAP_GET(characterData, EASY_STRING("packageId"))) : FGuid(),
			characterData.Contains(EASY_STRING("packageName"))
			? SAFE_MAP_GET(characterData, EASY_STRING("packageName")).AsString() : EMPTY_STRING));
	}
	return MakeUnique<ServerResponseCharacterList>(chars);
}

TUniquePtr<ServerResponseContextUpdated> VoxtaApiResponseHandler::GetContextUpdatedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	FString contextValue = FString();
	ProcessContextData(serverResponseData, contextValue);

	return MakeUnique<ServerResponseContextUpdated>(
		contextValue,
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("sessionId"))));
}

TUniquePtr<ServerResponseChatStarted> VoxtaApiResponseHandler::GetChatStartedResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	// Participants
	TMap<FString, FSignalRValue> user = SAFE_MAP_GET(serverResponseData, EASY_STRING("user")).AsObject();
	TArray<FSignalRValue> charIdArray = SAFE_MAP_GET(serverResponseData, EASY_STRING("characters")).AsArray();
	TArray<FGuid> chars;
	chars.Reserve(charIdArray.Num());
	for (const FSignalRValue& charElement : charIdArray)
	{
		const TMap<FString, FSignalRValue>& characterData = charElement.AsObject();
		chars.Emplace(GetStringAsGuid(SAFE_MAP_GET(characterData, EASY_STRING("id"))));
	}

	// Services
	TMap<FString, FSignalRValue> servicesMap = SAFE_MAP_GET(serverResponseData, EASY_STRING("services")).AsObject();
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
			services.Emplace(enumType, FVoxtaServiceData(enumType, SAFE_MAP_GET(serviceData, EASY_STRING("serviceName")).AsString(),
				GetStringAsGuid(SAFE_MAP_GET(serviceData, EASY_STRING("serviceId")))));
		}
	}

	// Context
	TMap<FString, FSignalRValue> context = SAFE_MAP_GET(serverResponseData, EASY_STRING("context")).AsObject();
	FString contextValue = FString();
	ProcessContextData(context, contextValue);

	return MakeUnique<ServerResponseChatStarted>(GetStringAsGuid(SAFE_MAP_GET(user, EASY_STRING("id"))),
		chars, services, GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("chatId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("sessionId"))),
		contextValue);
}

TUniquePtr<ServerResponseChatMessageStart> VoxtaApiResponseHandler::GetReplyStartReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	return MakeUnique<ServerResponseChatMessageStart>(
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("messageId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("senderId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("sessionId"))));
}

TUniquePtr<ServerResponseChatMessageChunk> VoxtaApiResponseHandler::GetReplyChunkReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	return MakeUnique<ServerResponseChatMessageChunk>(
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("messageId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("senderId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("sessionId"))),
		static_cast<int>(SAFE_MAP_GET(serverResponseData, EASY_STRING("startIndex")).AsDouble()),
		static_cast<int>(SAFE_MAP_GET(serverResponseData, EASY_STRING("endIndex")).AsDouble()),
		SAFE_MAP_GET(serverResponseData, EASY_STRING("text")).AsString(),
		SAFE_MAP_GET(serverResponseData, EASY_STRING("audioUrl")).AsString(),
		SAFE_MAP_GET(serverResponseData, EASY_STRING("isNarration")).AsBool());
}

TUniquePtr<ServerResponseChatMessageEnd> VoxtaApiResponseHandler::GetReplyEndReponseResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	return MakeUnique<ServerResponseChatMessageEnd>(
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("messageId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("senderId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("sessionId"))));
}

TUniquePtr<ServerResponseChatMessageCancelled> VoxtaApiResponseHandler::GetReplyCancelledResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	return MakeUnique<ServerResponseChatMessageCancelled>(
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("messageId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("sessionId"))));
}

TUniquePtr<ServerResponseChatUpdate> VoxtaApiResponseHandler::GetChatUpdateResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	return MakeUnique<ServerResponseChatUpdate>(
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("messageId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("senderId"))),
		SAFE_MAP_GET(serverResponseData, EASY_STRING("text")).AsString(),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("sessionId"))));
}

TUniquePtr<ServerResponseChatClosed> VoxtaApiResponseHandler::GetChatClosedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData)
{
	return MakeUnique<ServerResponseChatClosed>(
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("chatId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("sessionId"))));
}

TUniquePtr<ServerResponseSpeechTranscription> VoxtaApiResponseHandler::GetSpeechRecognitionPartial(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	return MakeUnique<ServerResponseSpeechTranscription>(
		SAFE_MAP_GET(serverResponseData, EASY_STRING("text")).AsString(),
		ServerResponseSpeechTranscription::TranscriptionState::PARTIAL);
}

TUniquePtr<ServerResponseSpeechTranscription> VoxtaApiResponseHandler::GetSpeechRecognitionEnd(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	bool isValid = serverResponseData.Contains(EASY_STRING("text"));
	return MakeUnique<ServerResponseSpeechTranscription>(
		isValid ? SAFE_MAP_GET(serverResponseData, EASY_STRING("text")).AsString()
		: FString(),
		isValid ? ServerResponseSpeechTranscription::TranscriptionState::END
		: ServerResponseSpeechTranscription::TranscriptionState::CANCELLED);
}

TUniquePtr<ServerResponseError> VoxtaApiResponseHandler::GetErrorResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	return MakeUnique<ServerResponseError>(
		SAFE_MAP_GET(serverResponseData, EASY_STRING("message")).AsString(),
		SAFE_MAP_GET(serverResponseData, EASY_STRING("details")).AsString());
}

void VoxtaApiResponseHandler::ProcessContextData(TMap<FString, FSignalRValue> contextMainObject,
	FString& outContextValue)
{
	//TArray<FSignalRValue> flagsArray = SAFE_MAP_GET(contextMainObject, EASY_STRING("flags")).AsArray();
	TArray<FSignalRValue> contextsArray = SAFE_MAP_GET(contextMainObject, EASY_STRING("contexts")).AsArray();
	//TArray<FSignalRValue> actionsArray = SAFE_MAP_GET(contextMainObject, EASY_STRING("actions")).AsArray();
	//TArray<FSignalRValue> charactersArray = SAFE_MAP_GET(contextMainObject, EASY_STRING("characters")).AsArray();
	//TArray<FSignalRValue> rolesArray = SAFE_MAP_GET(contextMainObject, EASY_STRING("roles")).AsArray();

	outContextValue = FString();

	for (const FSignalRValue& context : contextsArray)
	{
		const TMap<FString, FSignalRValue>& contextData = context.AsObject();
		if (SAFE_MAP_GET(contextData, EASY_STRING("contextKey")).AsString() == VOXTA_CONTEXT_KEY)
		{
			outContextValue = SAFE_MAP_GET(contextData, EASY_STRING("text")).AsString();
		}
	}
}

FGuid VoxtaApiResponseHandler::GetStringAsGuid(const FSignalRValue& input)
{
	return GetStringAsGuid(input.AsString());
}

FGuid VoxtaApiResponseHandler::GetStringAsGuid(const FString& input)
{
	FGuid userID;
	bool success = FGuid::Parse(input, userID);
	return userID;
}