// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncDataBase.h"
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
	ULipSyncDataCustom() : ILipSyncDataBase(LipSyncType::Custom)
	{
	};
#pragma endregion

#pragma region ILipSyncDataBase overrides
public:
	virtual void CleanupData() override
	{
	}
#pragma endregion
};
