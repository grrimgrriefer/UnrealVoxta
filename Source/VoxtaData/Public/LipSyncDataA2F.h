// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncDataBase.h"
#include "LipSyncDataA2F.generated.h"

UCLASS()
class VOXTADATA_API ULipSyncDataA2F : public UObject, public ILipSyncDataBase
{
	GENERATED_BODY()

public:
	ULipSyncDataA2F() : ILipSyncDataBase(LipSyncType::Audio2Face)
	{
	}

	//~ Start ILipSyncDataBase overrides
public:
	virtual void CleanupData() override
	{
	}
	//~ End ILipSyncDataBase overrides

protected:
};
