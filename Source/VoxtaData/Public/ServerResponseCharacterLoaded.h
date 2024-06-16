// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

struct ServerResponseCharacterLoaded : public ServerResponseBase
{
	const FString m_characterId;
	const bool m_enableThinkingSpeech;

	explicit ServerResponseCharacterLoaded(FString characterId,
			bool enableThinkingSpeech) :
		m_characterId(characterId),
		m_enableThinkingSpeech(enableThinkingSpeech)
	{
	}

	ServerResponseType GetType() final
	{
		return ServerResponseType::CharacterLoaded;
	}
};
