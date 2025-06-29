// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiResponseHandler.h"
#include "Logging/StructuredLog.h"
#include "SignalR/Public/SignalRValue.h"
#include "VoxtaData/Public/ServerResponses.h"
#include "VoxtaData/Public/VoxtaServiceEntryData.h"

#define SAFE_MAP_GET(Map, Key)                                               \
    ([&]() -> const FSignalRValue&                                           \
    {                                                                        \
        const auto& _key = (Key);                                            \
        if ((Map).Contains(_key))                                            \
        {                                                                    \
            return (Map)[_key];                                              \
        }                                                                    \
        UE_LOGFMT(VoxtaLog, Error, "Map missing key: {0}", *(_key));         \
        static const FSignalRValue _empty = FSignalRValue(EASY_STRING(""));  \
        return _empty;                                                       \
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
		EASY_STRING("memoryUpdated"),
		EASY_STRING("moduleRuntimeInstances"),
		EASY_STRING("inspectorEnabled")
};

const TMap<FString, TFunction<TUniquePtr<ServerResponseBase>(const TMap<FString, FSignalRValue>&)>> VoxtaApiResponseHandler::HANDLERS{
	{ TEXT("welcome"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetWelcomeResponse, data); } },
	{ TEXT("charactersListLoaded"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetCharacterListLoadedResponse, data); } },
	{ TEXT("chatStarted"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetChatStartedResponse, data); } },
	{ TEXT("replyStart"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetReplyStartResponse, data); } },
	{ TEXT("replyChunk"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetReplyChunkResponse, data); } },
	{ TEXT("replyEnd"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetReplyEndResponse, data); } },
	{ TEXT("replyCancelled"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetReplyCancelledResponse, data); } },
	{ TEXT("update"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetChatUpdateResponse, data); } },
	{ TEXT("speechRecognitionPartial"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetSpeechRecognitionPartial, data); } },
	{ TEXT("speechRecognitionEnd"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetSpeechRecognitionEnd, data); } },
	{ TEXT("error"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetErrorResponse, data); } },
	{ TEXT("chatSessionError"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetChatSessionErrorResponse, data); } },
	{ TEXT("contextUpdated"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetContextUpdatedResponse, data); } },
	{ TEXT("chatClosed"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetChatClosedResponse, data); } },
	{ TEXT("configuration"), [] (const auto& data) { return WrapHandler(&VoxtaApiResponseHandler::GetConfigurationResponse, data); } }
};

TUniquePtr<ServerResponseBase> VoxtaApiResponseHandler::GetResponseData(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	const FString type = SAFE_MAP_GET(serverResponseData, EASY_STRING("$type")).AsString();
	auto* handler = HANDLERS.Find(type);
	if (handler != nullptr)
	{
		return (*handler)(serverResponseData);
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
	TMap<VoxtaServiceType, FVoxtaServiceEntryData> services;
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
			services.Emplace(enumType, FVoxtaServiceEntryData(enumType, SAFE_MAP_GET(serviceData, EASY_STRING("serviceName")).AsString(),
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

TUniquePtr<ServerResponseChatMessageStart> VoxtaApiResponseHandler::GetReplyStartResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	return MakeUnique<ServerResponseChatMessageStart>(
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("messageId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("senderId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("sessionId"))));
}

TUniquePtr<ServerResponseChatMessageChunk> VoxtaApiResponseHandler::GetReplyChunkResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	return MakeUnique<ServerResponseChatMessageChunk>(
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("messageId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("senderId"))),
		GetStringAsGuid(SAFE_MAP_GET(serverResponseData, EASY_STRING("sessionId"))),
		static_cast<int>(SAFE_MAP_GET(serverResponseData, EASY_STRING("startIndex")).AsNumber()),
		static_cast<int>(SAFE_MAP_GET(serverResponseData, EASY_STRING("endIndex")).AsNumber()),
		SAFE_MAP_GET(serverResponseData, EASY_STRING("text")).AsString(),
		SAFE_MAP_GET(serverResponseData, EASY_STRING("audioUrl")).AsString(),
		SAFE_MAP_GET(serverResponseData, EASY_STRING("isNarration")).AsBool());
}

TUniquePtr<ServerResponseChatMessageEnd> VoxtaApiResponseHandler::GetReplyEndResponse(
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
		ServerResponseSpeechTranscription::TranscriptionState::Partial);
}

TUniquePtr<ServerResponseSpeechTranscription> VoxtaApiResponseHandler::GetSpeechRecognitionEnd(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	bool isValid = serverResponseData.Contains(EASY_STRING("text"));
	return MakeUnique<ServerResponseSpeechTranscription>(
		isValid ? SAFE_MAP_GET(serverResponseData, EASY_STRING("text")).AsString()
		: FString(),
		isValid ? ServerResponseSpeechTranscription::TranscriptionState::End
		: ServerResponseSpeechTranscription::TranscriptionState::Cancelled);
}

TUniquePtr<ServerResponseError> VoxtaApiResponseHandler::GetErrorResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	return MakeUnique<ServerResponseError>(
		SAFE_MAP_GET(serverResponseData, EASY_STRING("message")).AsString(),
		SAFE_MAP_GET(serverResponseData, EASY_STRING("details")).AsString());
}

TUniquePtr<ServerResponseChatSessionError> VoxtaApiResponseHandler::GetChatSessionErrorResponse(
	const TMap<FString, FSignalRValue>& serverResponseData)
{
	return MakeUnique<ServerResponseChatSessionError>(
		SAFE_MAP_GET(serverResponseData, EASY_STRING("sessionId")).AsString(),
		SAFE_MAP_GET(serverResponseData, EASY_STRING("retry")).AsBool(),
		SAFE_MAP_GET(serverResponseData, EASY_STRING("message")).AsString());
}

TUniquePtr<ServerResponseConfiguration> VoxtaApiResponseHandler::GetConfigurationResponse(
		const TMap<FString, FSignalRValue>& serverResponseData)
{
	// Services
	TMap<FString, FSignalRValue> servicesMap = SAFE_MAP_GET(serverResponseData, EASY_STRING("services")).AsObject();
	using enum VoxtaServiceType;
	TArray<FVoxtaServiceGroupData> serviceGroups;
	TMap<VoxtaServiceType, FString> serviceTypes = {
		{ TextGen, TEXT("TextGen") },
		{ SpeechToText, TEXT("SpeechToText") },
		{ TextToSpeech, TEXT("TextToSpeech") },
		{ ActionInference, TEXT("ActionInference") }
	};
	for (const auto& [enumType, stringValue] : serviceTypes)
	{
		if (servicesMap.Contains(stringValue))
		{
			const TMap<FString, FSignalRValue>& serviceGroupData = servicesMap[stringValue].AsObject();
			TArray<FSignalRValue> servicesEntriesOrigin = SAFE_MAP_GET(serviceGroupData, EASY_STRING("services")).AsArray();

			TArray<FVoxtaServiceEntryData> serviceEntries;
			serviceEntries.Reserve(servicesMap.Num());

			for (const FSignalRValue& serviceEntryElement : servicesEntriesOrigin)
			{
				const TMap<FString, FSignalRValue>& entryData = serviceEntryElement.AsObject();
				serviceEntries.Emplace(FVoxtaServiceEntryData(enumType, 
					SAFE_MAP_GET(entryData, EASY_STRING("serviceName")).AsString(),
					GetStringAsGuid(SAFE_MAP_GET(entryData, EASY_STRING("serviceId")))));
			}
			serviceGroups.Emplace(FVoxtaServiceGroupData(enumType, GetStringAsGuid(SAFE_MAP_GET(serviceGroupData, EASY_STRING("defaultServiceId"))), serviceEntries));
		}
	}

	return MakeUnique<ServerResponseConfiguration>(serviceGroups);
}

void VoxtaApiResponseHandler::ProcessContextData(const TMap<FString, FSignalRValue>& contextMainObject,
	FString& outContextValue)
{
	outContextValue = FString();

	//TArray<FSignalRValue> flagsArray = SAFE_MAP_GET(contextMainObject, EASY_STRING("flags")).AsArray();
	//TArray<FSignalRValue> actionsArray = SAFE_MAP_GET(contextMainObject, EASY_STRING("actions")).AsArray();
	//TArray<FSignalRValue> charactersArray = SAFE_MAP_GET(contextMainObject, EASY_STRING("characters")).AsArray();
	//TArray<FSignalRValue> rolesArray = SAFE_MAP_GET(contextMainObject, EASY_STRING("roles")).AsArray();

	if (!contextMainObject.Contains(EASY_STRING("contexts")))
	{
		return;
	}

	TArray<FSignalRValue> contextsArray = SAFE_MAP_GET(contextMainObject, EASY_STRING("contexts")).AsArray();
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