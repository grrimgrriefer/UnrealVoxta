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
		{ EASY_STRING("scope"), FSignalRValue(TArray<FSignalRValue> { SIGNALR_STRING("role:app") }) },
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

FSignalRValue VoxtaApiRequestHandler::GetLoadScenariosListData() const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("loadScenariosList") }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetLoadChatsListData(const FGuid& characterId, const FGuid& scenarioId) const
{
	TMap<FString, FSignalRValue> requestData = TMap<FString, FSignalRValue>{
		{ EASY_STRING("$type"), SIGNALR_STRING("loadChatsList") },
		{ EASY_STRING("characterId"), FSignalRValue(GuidToString(characterId)) }
	};

	if (scenarioId.IsValid())
	{
		requestData.Add(
		{
			EASY_STRING("scenarioId"), FSignalRValue(GuidToString(scenarioId))
		});
	}

	return FSignalRValue(requestData);
}

FSignalRValue VoxtaApiRequestHandler::GetStartChatRequestData(const FAiCharData* charData, const FString& context) const
{
	TMap<FString, FSignalRValue> requestData = TMap<FString, FSignalRValue>{
		{ EASY_STRING("$type"), SIGNALR_STRING("startChat") },
		{ EASY_STRING("contextKey"), FSignalRValue(VOXTA_CONTEXT_KEY) },
		{ EASY_STRING("characterIds"), FSignalRValue(TArray<FSignalRValue> { FSignalRValue(GuidToString(charData->GetId())) }) }
	};

	if (!context.IsEmpty())
	{
		requestData.Add({ EASY_STRING("contexts"), FSignalRValue(TArray<FSignalRValue> {
			FSignalRValue(TMap<FString, FSignalRValue> {
				{ EASY_STRING("text"), context }
			})
		}) });
	}

	// Req for action inference TODO
	// { EASY_STRING("actions"), ... }

	// Req for scenario support TODO
	// { EASY_STRING("scenarioId"), ... }

	// Req for multi-character support (possibly also scenarios with multiple characters?) TODO
	// { EASY_STRING("roles"), ... }

	// Not sure why starting a chat would require a chat id? maybe to continue? Need to be figured out TODO
	// { EASY_STRING("chatId"), ... }

	// Not sure what chat dependencies are, need to figure it out TODO
	// { EASY_STRING("dependencies"), ... }

	return FSignalRValue(requestData);
}

FSignalRValue VoxtaApiRequestHandler::GetStopChatRequestData() const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("stopChat") }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetSendUserMessageData(const FGuid& sessionId,
	const FString& userInputText, bool generateReply, bool characterActionInference) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("send") },
		{ EASY_STRING("sessionId"), FSignalRValue(GuidToString(sessionId)) },
		{ EASY_STRING("text"), FSignalRValue(userInputText) },
		{ EASY_STRING("doReply"), generateReply ? SIGNALR_STRING("true") : SIGNALR_STRING("false") },
		{ EASY_STRING("doCharacterActionInference"), characterActionInference ? SIGNALR_STRING("true") : SIGNALR_STRING("false") }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetNotifyAudioPlaybackCompletedData(const FGuid& sessionId,
	const FGuid& messageId) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("speechPlaybackComplete") },
		{ EASY_STRING("sessionId"), FSignalRValue(GuidToString(sessionId)) },
		{ EASY_STRING("messageId"), FSignalRValue(GuidToString(messageId)) }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetNotifyAudioPlaybackStartedData(const FGuid& sessionId,
	const FGuid& messageId, int startIndex, int endIndex, double duration) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("speechPlaybackStart") },
		{ EASY_STRING("sessionId"), FSignalRValue(GuidToString(sessionId)) },
		{ EASY_STRING("messageId"), FSignalRValue(GuidToString(messageId)) },
		{ EASY_STRING("startIndex"), FSignalRValue(startIndex) },
		{ EASY_STRING("endIndex"), FSignalRValue(endIndex) },
		{ EASY_STRING("duration"), FSignalRValue(FString::Printf(TEXT("%.3f"), duration)) }
	});
}

FSignalRValue VoxtaApiRequestHandler::GetUpdateContextRequestData(const TArray<FString>& actions,
	const FGuid& sessionId, const FString& context, const FString& contextKey) const
{
	TMap<FString, FSignalRValue> requestData = TMap<FString, FSignalRValue>{
		{ EASY_STRING("$type"), SIGNALR_STRING("updateContext") },
		{ EASY_STRING("sessionId"), FSignalRValue(GuidToString(sessionId)) },
		{ EASY_STRING("contextKey"), FSignalRValue(contextKey) }
	};

	if (!context.IsEmpty())
	{
		requestData.Add({ EASY_STRING("contexts"), FSignalRValue(TArray<FSignalRValue> {
			FSignalRValue(TMap<FString, FSignalRValue> {
				{ EASY_STRING("text"), context }
			})
		}) });
	}
	else
	{
		requestData.Add({ EASY_STRING("contexts"), FSignalRValue(TArray<FSignalRValue>()) });
	}

	// Req for action inference TODO
	// { EASY_STRING("actions"), ... }

	return FSignalRValue(requestData);
}

FSignalRValue VoxtaApiRequestHandler::GetRequestCharacterSpeechRequestData(const FGuid& sessionId,
	const FString& text) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("characterSpeechRequest") },
		{ EASY_STRING("sessionId"), FSignalRValue(GuidToString(sessionId)) },
		{ EASY_STRING("text"), FSignalRValue(text) }
	});
}

FSignalRValue VoxtaApiRequestHandler::SendRevertLastSentMessage(const FGuid& sessionId) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("revert") },
		{ EASY_STRING("sessionId"), FSignalRValue(GuidToString(sessionId)) }
	});
}

FSignalRValue VoxtaApiRequestHandler::SendDeleteChat(const FGuid& chatId) const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("deleteChat") },
		{ EASY_STRING("chatId"), FSignalRValue(GuidToString(chatId)) }
	});
}

FSignalRValue VoxtaApiRequestHandler::SetFlags(const FGuid& sessionId, const TArray<FString>& flags) const
{
	TArray<FSignalRValue> serializedFlags = TArray<FSignalRValue>();
	for (int i = 0; i < flags.Num(); i++)
	{
		serializedFlags.Add(FSignalRValue(flags[i]));
	}

	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ EASY_STRING("$type"), SIGNALR_STRING("updateContext") },
		{ EASY_STRING("sessionId"), FSignalRValue(GuidToString(sessionId)) },
		{ EASY_STRING("setFlags"), FSignalRValue(serializedFlags) }
	});
}