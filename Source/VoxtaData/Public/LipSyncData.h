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

USTRUCT()
struct FLipSyncData
{
	GENERATED_BODY()

public:
	FLipSyncData()
	{
		m_id = FGuid::NewGuid();
		m_lipsyncType = LipSyncType::Custom;
	};

#if WITH_OVRLIPSYNC
public:
	FLipSyncData(UOVRLipSyncFrameSequence* ovrLipSyncFrameSequence)
	{
		m_ovrLipSyncFrameSequence = ovrLipSyncFrameSequence;
		m_id = FGuid::NewGuid();
		m_lipsyncType = LipSyncType::OVRLipSync;
	}
#endif

	void CleanupData()
	{
#if WITH_OVRLIPSYNC
		if (m_lipsyncType == LipSyncType::OVRLipSync)
		{
			m_ovrLipSyncFrameSequence->RemoveFromRoot();
		}
#endif
	}

	FGuid GetGuid() const
	{
		return m_id;
	}

private:
	FGuid m_id;
	LipSyncType m_lipsyncType;

#if WITH_OVRLIPSYNC
public:
	UOVRLipSyncFrameSequence* GetOvrLipSyncData() const { return m_ovrLipSyncFrameSequence; }

protected:
	UOVRLipSyncFrameSequence* m_ovrLipSyncFrameSequence;
#endif
};