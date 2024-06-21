// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaDefines.h"
#include "SignalRValue.h"
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
#include "VoxtaData/Public/VoxtaServiceData.h"

class VoxtaApiResponseHandler
{
public:
	// Currently ignored messages
	const TSet<FString> ignoredMessageTypes{
		API_STRING("chatStarting"),
		API_STRING("chatLoadingMessage"),
		API_STRING("chatsSessionsUpdated"),
		API_STRING("contextUpdated"),
		API_STRING("replyGenerating"),
		API_STRING("chatFlow"),
		API_STRING("speechRecognitionStart"),
		API_STRING("recordingRequest"),
		API_STRING("recordingStatus"),
		API_STRING("speechPlaybackComplete"),
		API_STRING("memoryUpdated")
	};

	TUniquePtr<IServerResponseBase> GetResponseData(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

private:
	TUniquePtr<ServerResponseWelcome> GetWelcomeResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseCharacterList> GetCharacterListLoadedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseCharacterLoaded> GetCharacterLoadedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatStarted> GetChatStartedResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatMessageStart> GetReplyStartReponseResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatMessageChunk> GetReplyChunkReponseResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatMessageEnd> GetReplyEndReponseResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatMessageCancelled> GetReplyCancelledResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatUpdate> GetChatUpdateResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;
};
