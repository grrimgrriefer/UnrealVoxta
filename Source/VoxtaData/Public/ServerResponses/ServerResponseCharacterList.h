// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "AiCharData.h"

/// <summary>
/// Read-only data struct containing the relevant data of the 'charactersListLoaded' response
/// from the VoxtaServer.
/// </summary>
struct ServerResponseCharacterList : public IServerResponseBase
{
public:
	const TArray<FAiCharData> m_characters;

	explicit ServerResponseCharacterList(const TArray<FAiCharData>& characters) :
		m_characters(characters)
	{
	}

	///<inheritdoc />
	ServerResponseType GetType() const final
	{
		return ServerResponseType::CharacterList;
	}
};
