// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "CharData.h"

struct ServerResponseCharacterList : public ServerResponseBase
{
	const TArray<FCharData> m_characters;

	explicit ServerResponseCharacterList(const TArray<FCharData>& characters) :
		m_characters(characters)
	{
	}

	ServerResponseType GetType() final
	{
		return ServerResponseType::CharacterList;
	}
};
