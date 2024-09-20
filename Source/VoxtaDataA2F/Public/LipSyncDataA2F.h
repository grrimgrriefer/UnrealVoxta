// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncBaseData.h"
#include "LipSyncDataA2F.generated.h"

UCLASS()
class VOXTADATAA2F_API ULipSyncDataA2F : public UObject, public ILipSyncBaseData
{
	GENERATED_BODY()

public:
	ULipSyncDataA2F() : ILipSyncBaseData(LipSyncType::Audio2Face)
	{
	}

	void SetA2FCurveWeights(const TArray<TArray<float>>& sourceCurves, int framesPerSecond)
	{
		m_completeSampleCount = sourceCurves;
		m_framesPerSecond = framesPerSecond;
	}

	int GetFramePerSecond() const
	{
		return m_framesPerSecond;
	}

	const TArray<TArray<float>>& GetA2FCurveWeights() const
	{
		return m_completeSampleCount;
	}

	//~ Start ILipSyncBaseData overrides
public:
	virtual void CleanupData() override
	{
	}
	//~ End ILipSyncBaseData overrides

protected:
	int m_framesPerSecond;
	TArray<TArray<float>> m_completeSampleCount;
};
