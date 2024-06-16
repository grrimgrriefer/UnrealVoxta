// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "SignalRValue.h"
#include "VoxtaData/Public/CharData.h"
#include "Misc/Guid.h"

class VoxtaApiRequestHandler
{
public:
	FSignalRValue GetAuthenticateRequestData() const;
	FSignalRValue GetLoadCharactersListData() const;
	FSignalRValue GetLoadCharacterRequestData(const FString& characterId) const;
	FSignalRValue GetStartChatRequestData(const FCharData* charData) const;
};
