// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncType.h"
#include "LipSyncDataBase.generated.h"

/**
 * ULipSyncDataBase
 * Used by UE to ensure the ILipSyncDataBase interface is picked up by Unreal's Reflection system
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class ULipSyncDataBase : public UInterface
{
	GENERATED_BODY()
};

/**
 * ILipSyncDataBase
 * The interface implemented by whoever should hold the LipSync related data & be responsible for cleaning it up.
 *
 * Note: This instance only contains the data for a single VoiceLine. It is not recycled, nor is it guaranteed to
 * contain all the lipsync data for the entire ChatMessage.
 * In practice this is used to ensure the VoxtaAudioPlayback and (most of ) the MessageChunkAudioContainer
 * doesn't need to know which type of lipsync is used.
 */
class ILipSyncDataBase
{
	GENERATED_BODY()

public:
#pragma region public API
	ILipSyncDataBase()
	{
		m_id = FGuid::NewGuid();
		m_lipsyncType = LipSyncType::None;
	};

	ILipSyncDataBase(LipSyncType lipSyncType)
	{
		m_id = FGuid::NewGuid();
		m_lipsyncType = lipSyncType;
	};

	/**
	 * Clean up the data that was made / kept that was directly tied to the playback of one voiceline.
	 * Once this is called all memory will be cleaned and the playback can no longer be done.
	 */
	virtual void CleanupData() = 0;

	/**
	 * Every instance of a type deriving from this interface assigns a Guid to itself.
	 * This can be used to keep track of it, but a direct reference is also allowed. This just makes life easier.
	 *
	 * @return A unique Guid that was generated upon the construction of this instance.
	 */
	FGuid GetGuid() const
	{
		return m_id;
	}
#pragma endregion

#pragma region data
private:
	FGuid m_id;
	LipSyncType m_lipsyncType;
#pragma endregion
};
