// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncBaseData.h"
#include "LipSyncDataA2F.generated.h"

/**
 * ULipSyncDataA2F.
 * Wrapper for A2F-lipsync specific data. A2F logic is it's own module for SOLID principle, not for any
 * dependency reasons. (A2F just needs a HTTP client internally to work)
 */
UCLASS(Category = "Voxta")
class VOXTAUTILITY_A2F_API ULipSyncDataA2F : public UObject, public ILipSyncBaseData
{
	GENERATED_BODY()

#pragma region ILipSyncBaseData overrides
public:
	/**
	 * Clean up the A2F-lipsync data (currently nothing)
	 */
	virtual void ReleaseData() override
	{
		this->RemoveFromRoot();
	}
#pragma endregion

#pragma region public API
public:
	/** Create an instance of the LipSyncData holder for Audio2Face. */
	ULipSyncDataA2F() : ILipSyncBaseData()
	{
	}

	/**
	 * Register the genereated curves from A2F as part of this data object.
	 * These are returned by the A2F_headless REST api.
	 *
	 * @param sourceCurves The ARKit curve values per setting per frame.
	 * @param framesPerSecond The fps that this data was generated at (high framerates will interpolate)
	 */
	void SetA2FCurveWeights(const TArray<TArray<float>>& sourceCurves, int framesPerSecond)
	{
		m_completeSampleCount = sourceCurves;
		m_framesPerSecond = framesPerSecond;
	}

	/** @return The FPS that the curves were genrated at. */
	int GetFramePerSecond() const
	{
		return m_framesPerSecond;
	}

	/** @return A direct reference to the generated curve weights. */
	const TArray<TArray<float>>& GetA2FCurveWeights() const
	{
		return m_completeSampleCount;
	}

#pragma region data
private:
	int m_framesPerSecond;
	TArray<TArray<float>> m_completeSampleCount;
#pragma endregion
};
