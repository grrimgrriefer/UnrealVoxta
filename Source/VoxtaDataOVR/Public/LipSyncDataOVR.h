// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncBaseData.h"
#include "LipSyncDataOVR.generated.h"

class UOVRLipSyncFrameSequence;

UCLASS()
class VOXTADATAOVR_API ULipSyncDataOVR : public UObject, public ILipSyncBaseData
{
	GENERATED_BODY()

public:
	ULipSyncDataOVR() : ILipSyncBaseData(LipSyncType::OVRLipSync)
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

	//~ Start ILipSyncBaseData overrides
public:
	virtual void CleanupData() override
	{
		m_ovrLipSyncFrameSequence = nullptr;
		this->RemoveFromRoot();
	}
	//~ End ILipSyncBaseData overrides

protected:
	UPROPERTY()
	UOVRLipSyncFrameSequence* m_ovrLipSyncFrameSequence;
};
