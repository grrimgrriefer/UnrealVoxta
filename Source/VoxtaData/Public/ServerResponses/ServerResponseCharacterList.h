// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "AiCharData.h"

/**
 * Read-only data struct containing the relevant data of the 'charactersListLoaded' response from the VoxtaServer.
 */
struct ServerResponseCharacterList : public ServerResponseBase
{
#pragma region public API
public:
	/** Create a deserialized version of the VoxtaServer response represents the 'CharacterList' data. */
	explicit ServerResponseCharacterList(const TArray<FAiCharData>& characters) :
		ServerResponseBase(ServerResponseType::CharacterList),
		CHARACTERS(characters)
	{
	}
#pragma endregion

#pragma region data
public:
	const TArray<FAiCharData> CHARACTERS;
#pragma endregion
};
