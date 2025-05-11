// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncBaseData.h"
#include "LipSyncDataCustom.generated.h"

/**
 * ULipSyncDataCustom
 * UObject-based container for CustomLipSync lipsync data.
 * Holds all data required for playback of custom lipsync generation.
 * Keeps lipsync logic modular and consistent between all lipsync types.
 * Each instance holds the lipsync data for a single voiceline and is responsible for its own cleanup.
 */
UCLASS(Category = "Voxta")
class VOXTADATA_API ULipSyncDataCustom : public UObject, public ILipSyncBaseData
{
	GENERATED_BODY()

#pragma region ILipSyncBaseData overrides
public:
	/**
	 * Clean up the data that was made / kept that was directly tied to the playback of one voiceline.
	 * Removes this object from the root set, allowing it to be garbage collected.
	 * Should be called when playback is finished and the data is no longer needed.
	 */
	virtual void ReleaseData() override
	{
		RemoveFromRoot();
	}
#pragma endregion

#pragma region public API
public:
	/**
	 * Constructor for the CustomLipSync data holder.
	 * Adds this object to the root set to prevent garbage collection during playback.
	 */
	ULipSyncDataCustom() : ILipSyncBaseData()
	{
		AddToRoot();
	}
#pragma endregion
};
