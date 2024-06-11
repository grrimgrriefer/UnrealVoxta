// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaApiResponseHandler.h"
#include "VoxtaLogUtility.h"

TUniquePtr<ServerResponseBase> VoxtaApiResponseHandler::GetResponseData(
	const TMap<FString, FSignalRValue>& serverResponseData) const
{
	using enum ServerResponseType;
	const FString type = serverResponseData[API_STRING("$type")].AsString();
	if (type == "welcome")
	{
		return GetWelcomeResponse(serverResponseData);
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
		CharData(user[API_STRING("id")].AsString(), user[API_STRING("name")].AsString()));
}