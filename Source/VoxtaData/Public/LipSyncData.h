// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#if WITH_OVRLIPSYNC
#include "OVRLipSyncFrame.h"
#endif
#include "LipSyncData.generated.h"

UENUM(BlueprintType)
enum class LipSyncType : uint8
{
	None				UMETA(DisplayName = "None"),
	Custom				UMETA(DisplayName = "Custom"),
	OVRLipSync			UMETA(DisplayName = "OVRLipSync")
};

USTRUCT(BlueprintType)
struct FLipSyncData
{
	GENERATED_BODY()

public:
	void CleanupData()
	{
#if WITH_OVRLIPSYNC
		m_ovrLipSyncFrameSequence->RemoveFromRoot();
#endif
	}

#if WITH_OVRLIPSYNC
public:
	void SetLipsyncData(UOVRLipSyncFrameSequence* ovrLipSyncFrameSequence)
	{
		m_ovrLipSyncFrameSequence = ovrLipSyncFrameSequence;
	}

	UOVRLipSyncFrameSequence* GetOvrLipSyncData() const { return m_ovrLipSyncFrameSequence; }

protected:
	UOVRLipSyncFrameSequence* m_ovrLipSyncFrameSequence;
#endif
};