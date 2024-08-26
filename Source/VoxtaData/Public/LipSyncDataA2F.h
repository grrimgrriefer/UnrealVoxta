﻿// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncDataBase.h"
#include "TimedWeightSample.h"
#include "LipSyncDataA2F.generated.h"

UCLASS()
class VOXTADATA_API ULipSyncDataA2F : public UObject, public ILipSyncDataBase
{
	GENERATED_BODY()

public:
	ULipSyncDataA2F() : ILipSyncDataBase(LipSyncType::Audio2Face)
	{
	}

	void SetA2FCurveWeights(const TArray<TimedWeightSample>& sourceCurves, int framesPerSecond)
	{
		m_completeSampleCount = sourceCurves;
		m_framesPerSecond = framesPerSecond;
	}

	int GetFramePerSecond() const
	{
		return m_framesPerSecond;
	}

	const TArray<TimedWeightSample>& GetA2FCurveWeights() const
	{
		return m_completeSampleCount;
	}

	//~ Start ILipSyncDataBase overrides
public:
	virtual void CleanupData() override
	{
	}
	//~ End ILipSyncDataBase overrides

protected:
	int m_framesPerSecond;
	TArray<TimedWeightSample> m_completeSampleCount;
};
