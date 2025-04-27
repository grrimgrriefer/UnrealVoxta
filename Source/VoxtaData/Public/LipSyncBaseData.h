// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncType.h"
#include "LipSyncBaseData.generated.h"

/**
 * ULipSyncBaseData
 * Used by UE to ensure the ILipSyncBaseData interface is picked up by Unreal's Reflection system
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class ULipSyncBaseData : public UInterface
{
	GENERATED_BODY()
};

/**
 * ILipSyncBaseData
 * The interface implemented by whoever should hold the LipSync related data & be responsible for cleaning it up.
 * Each MessageChunkAudioContainer should have one instance of a class deriving from this class (except if for
 * LipSyncType None)
 *
 * Note: Instances of this only contain the data for a single VoiceLine. It is not recycled, nor is it guaranteed to
 * contain all the lipsync data for the entire ChatMessage.
 *
 * In practice this is used to ensure the VoxtaAudioPlayback and (most of ) the MessageChunkAudioContainer
 * doesn't need to know which type of lipsync is used.
 */
class ILipSyncBaseData
{
	GENERATED_BODY()

#pragma region public API
public:
	/** Default constructor, should not be used manually, but is enforced by Unreal */
	explicit ILipSyncBaseData()
	{
		m_id = FGuid::NewGuid();
	}

	/**
	 * Clean up the data that was made / kept that was directly tied to the playback of one voiceline.
	 * Once this is called all memory will be cleaned and the playback can no longer be done.
	 */
	virtual void ReleaseData() = 0;

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
#pragma endregion
};
