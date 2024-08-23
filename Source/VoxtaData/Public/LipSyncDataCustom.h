// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncDataBase.h"
#include "LipSyncDataCustom.generated.h"

UCLASS()
class ULipSyncDataCustom : public UObject, public ILipSyncDataBase
{
	GENERATED_BODY()

public:
	ULipSyncDataCustom() : ILipSyncDataBase(LipSyncType::Custom)
	{
	};

	//~ Start ILipSyncDataBase overrides
public:
	virtual void CleanupData() override
	{
	}
	//~ End ILipSyncDataBase overrides
};
