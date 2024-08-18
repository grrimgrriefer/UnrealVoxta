// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncType.h"
#include "LipSyncDataBase.generated.h"

UINTERFACE(MinimalAPI)
class ULipSyncDataBase : public UInterface
{
	GENERATED_BODY()
};

class ILipSyncDataBase
{
	GENERATED_BODY()

public:
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

	virtual void CleanupData() = 0;

	FGuid GetGuid() const
	{
		return m_id;
	}

private:
	FGuid m_id;
	LipSyncType m_lipsyncType;
};
