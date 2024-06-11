// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "CharData.h"

struct ServerResponseCharacterList : public ServerResponseBase
{
	const TArray<CharData> m_characters;

	explicit ServerResponseCharacterList(const TArray<CharData>& characters) :
		m_characters(characters)
	{
	}

	ServerResponseType GetType() final
	{
		return ServerResponseType::CharacterList;
	}
};
