// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiRequestHandler.h"

FSignalRValue VoxtaApiRequestHandler::GetAuthenticateRequestData() const
{
	return FSignalRValue(TMap<FString, FSignalRValue> {
		{ FString(TEXT("$type")), FSignalRValue(FString(TEXT("authenticate"))) },
		{ FString(TEXT("client")), FSignalRValue(FString(TEXT("TalkToMeCppUE"))) },
		{ FString(TEXT("clientVersion")), FSignalRValue(FString(TEXT("0.0.1a"))) },
		{ FString(TEXT("scope")), FSignalRValue(TArray<FSignalRValue> { FSignalRValue(FString(TEXT("role:app"))), FSignalRValue(FString(TEXT("broadcast:write"))) }) },
		{ FString(TEXT("capabilities")), FSignalRValue(TMap<FString, FSignalRValue> {
			{ FString(TEXT("audioInput")),  FSignalRValue(FString(TEXT("WebSocketStream"))) },
			{ FString(TEXT("audioOutput")),  FSignalRValue(FString(TEXT("Url"))) },
			{ FString(TEXT("acceptedAudioContentTypes")), FSignalRValue(TArray<FSignalRValue> { FSignalRValue(FString(TEXT("audio/x-wav"))) }) }
		}) }
	});
}