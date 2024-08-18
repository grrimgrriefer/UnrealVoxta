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
		m_ovrLipSyncFrameSequence = nullptr;
	}

	ULipSyncDataOVR(UOVRLipSyncFrameSequence* ovrLipSyncFrameSequence) : ILipSyncDataBase(LipSyncType::OVRLipSync)
	{
		m_ovrLipSyncFrameSequence = ovrLipSyncFrameSequence;
	}

	virtual void CleanupData() override
	{
		m_ovrLipSyncFrameSequence->RemoveFromRoot();
	}

	UOVRLipSyncFrameSequence* GetOvrLipSyncData() const { return m_ovrLipSyncFrameSequence; }

protected:
	UPROPERTY()
	UOVRLipSyncFrameSequence* m_ovrLipSyncFrameSequence;
};
