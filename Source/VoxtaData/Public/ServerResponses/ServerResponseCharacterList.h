// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "AiCharData.h"

/**
 * ServerResponseCharacterList
 * Read-only data struct containing the relevant data of the 'charactersListLoaded' response from the VoxtaServer.
 * Contains a list of all available AI character data.
 */
struct ServerResponseCharacterList : public ServerResponseBase
{
#pragma region public API
public:
	/**
	 * Construct a character list response.
	 *
	 * @param characters The list of AI character data.
	 */
	explicit ServerResponseCharacterList(TArray<FAiCharData> characters) :
		ServerResponseBase(ServerResponseType::CharacterList),
		CHARACTERS(MoveTemp(characters))
	{}
#pragma endregion

#pragma region data
public:
	/** The list of AI character data. */
	const TArray<FAiCharData> CHARACTERS;
#pragma endregion
};
