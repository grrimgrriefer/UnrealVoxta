// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaDefines.h"
#include "SignalRValue.h"
#include "VoxtaData/Public/ServerResponseBase.h"
#include "VoxtaData/Public/ServerResponseWelcome.h"
#include "VoxtaData/Public/ServerResponseCharacterList.h"
#include "VoxtaData/Public/ServerResponseCharacterLoaded.h"
#include "VoxtaData/Public/ServerResponseChatStarted.h"
#include "VoxtaData/Public/ServerResponseChatMessage.h"
#include "VoxtaData/Public/ServerResponseChatUpdate.h"
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
		API_STRING("speechPlaybackComplete")
	};

	TUniquePtr<ServerResponseBase> GetResponseData(
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

	TUniquePtr<ServerResponseChatMessage> GetReplyStartReponseResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatMessage> GetReplyChunkReponseResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatMessage> GetReplyEndReponseResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;

	TUniquePtr<ServerResponseChatUpdate> GetChatUpdateResponse(
		const TMap<FString, FSignalRValue>& serverResponseData) const;
};
