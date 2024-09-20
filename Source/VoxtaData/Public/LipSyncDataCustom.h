// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncBaseBase.h"
#include "LipSyncDataCustom.generated.h"

/**
 * ULipSyncDataCustom
 * Contains all the data required for playback of CustomLipSync generation
 *
 * Technically not needed, but made to keep things consistent between all LipSync types. Otherwise using the
 * interface gets very messy.
 */
UCLASS()
class ULipSyncDataCustom : public UObject, public ILipSyncDataBase
{
	GENERATED_BODY()

#pragma region public API
public:
	/** Create an instance of the LipSyncData holder for CustomLipSync. */
	explicit ULipSyncDataCustom() : ILipSyncDataBase(LipSyncType::Custom)
	{
	};
#pragma endregion

#pragma region ILipSyncDataBase overrides
public:
	/**
	 * Clean up the data that was made / kept that was directly tied to the playback of one voiceline.
	 * Once this is called all memory will be cleaned and the playback can no longer be done.
	 */
	virtual void CleanupData() override
	{
	}
#pragma endregion
};
