// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/// <summary>
/// Read-only data struct containing the relevant data of the 'characterLoaded' response
/// from the VoxtaServer.
/// </summary>
struct ServerResponseCharacterLoaded : public IServerResponseBase
{
public:
	const FString m_characterId;
	const bool m_enableThinkingSpeech;

	explicit ServerResponseCharacterLoaded(FStringView characterId, bool enableThinkingSpeech) :
		m_characterId(characterId),
		m_enableThinkingSpeech(enableThinkingSpeech)
	{
	}

	/// <summary>
	/// Identifies the response type as CharacterLoaded.
	/// </summary>
	/// <returns>Returns MessageType::CharacterLoaded.</returns>
	ServerResponseType GetType() const final
	{
		return ServerResponseType::CharacterLoaded;
	}
};
