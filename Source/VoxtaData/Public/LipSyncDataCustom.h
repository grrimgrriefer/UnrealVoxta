// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncBaseData.h"
#include "LipSyncDataCustom.generated.h"

/**
 * ULipSyncDataCustom
 * Contains all the data required for playback of CustomLipSync generation
 *
 * Technically not needed, but made to keep things consistent between all LipSync types. Otherwise using the
 * interface gets very messy.
 */
UCLASS(Category = "Voxta")
class VOXTADATA_API ULipSyncDataCustom : public UObject, public ILipSyncBaseData
{
	GENERATED_BODY()

#pragma region public API
public:
	/** Create an instance of the LipSyncData holder for CustomLipSync. */
	explicit ULipSyncDataCustom() : ILipSyncBaseData(LipSyncType::Custom)
	{
	};
#pragma endregion

#pragma region ILipSyncBaseData overrides
public:
	/**
	 * Clean up the data that was made / kept that was directly tied to the playback of one voiceline.
	 * Once this is called all memory will be cleaned and the playback can no longer be done.
	 */
	virtual void ReleaseData() override
	{
	}
#pragma endregion
};
