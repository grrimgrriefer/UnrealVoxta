// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncType.h"
#include "LipSyncBaseData.generated.h"

/**
 * ULipSyncBaseData
 * Used by Unreal Engine to ensure the ILipSyncBaseData interface is recognized by Unreal's Reflection system.
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class ULipSyncBaseData : public UInterface
{
	GENERATED_BODY()
};

/**
 * ILipSyncBaseData
 * Interface for lipsync data containers. Implemented by all lipsync data types (A2F, Custom, etc).
 * Each instance holds the lipsync data for a single voiceline and is responsible for its own cleanup.
 */
class ILipSyncBaseData
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Default constructor. Assigns a unique GUID to this instance.
	 */
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
	 * Retrieves the unique GUID (FGuid) assigned to this instance.
	 * 
	 * @return The FGuid generated upon construction.
	 */
	FGuid GetGuid() const
	{
		return m_id;
	}
#pragma endregion

#pragma region data
private:
	/** Unique identifier for this lipsync data instance. */
	FGuid m_id;
#pragma endregion
};
