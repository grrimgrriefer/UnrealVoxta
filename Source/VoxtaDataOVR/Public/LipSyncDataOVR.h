// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "OVRLipSyncFrame.h"
#include "LipSyncDataBase.h"
#include "LipSyncDataOVR.generated.h"

UCLASS()
class VOXTADATAOVR_API ULipSyncDataOVR : public UObject, public ILipSyncDataBase
{
	GENERATED_BODY()

public:
	ULipSyncDataOVR() : ILipSyncDataBase(LipSyncType::OVRLipSync)
	{
	}

	void SetFrameSequence(UOVRLipSyncFrameSequence* ovrLipSyncFrameSequence)
	{
		m_ovrLipSyncFrameSequence = MoveTemp(ovrLipSyncFrameSequence);
	}

	UOVRLipSyncFrameSequence* GetOvrLipSyncData() const
	{
		return m_ovrLipSyncFrameSequence;
	}

	//~ Start ILipSyncDataBase overrides
public:
	virtual void CleanupData() override
	{
		m_ovrLipSyncFrameSequence = nullptr;
		this->RemoveFromRoot();
	}
	//~ End ILipSyncDataBase overrides

protected:
	UPROPERTY()
	UOVRLipSyncFrameSequence* m_ovrLipSyncFrameSequence;
};
