// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "ServerResponseBase.h"
//
///**
// * Read-only data struct containing the relevant data of the 'characterLoaded' response from the VoxtaServer.
// */
//
//struct ServerResponseCharacterLoaded : public ServerResponseBase
//{
//#pragma region public API
//public:
//	/** Create a deserialized version of the VoxtaServer response represents the 'CharacterLoaded' data. */
//	explicit ServerResponseCharacterLoaded(FStringView characterId, bool enableThinkingSpeech) :
//		ServerResponseBase(ServerResponseType::CharacterLoaded),
//		CHARACTER_ID(characterId),
//		ENABLE_THINKING_SPEECH(enableThinkingSpeech)
//	{
//	}
//#pragma endregion
//
//#pragma region data
//public:
//	const FString CHARACTER_ID;
//	const bool ENABLE_THINKING_SPEECH;
//#pragma endregion
//};