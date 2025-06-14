// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncBaseData.h"
#include "LipSyncDataA2F.generated.h"

/**
 * ULipSyncDataA2F
 * UObject-based container for Audio2Face lipsync data.
 * Holds all data required for playback of Audio2Face lipsync generation.
 * Keeps A2F logic modular and separated from other lipsync types.
 * Each instance holds the lipsync data for a single voiceline and is responsible for its own cleanup.
 */
UCLASS(Category = "Voxta")
class VOXTAUTILITY_A2F_API ULipSyncDataA2F : public UObject, public ILipSyncBaseData
{
	GENERATED_BODY()

#pragma region ILipSyncBaseData overrides
public:
	/**
	 * Clean up the A2F-lipsync data.
	 * Removes this object from the root set, allowing it to be garbage collected.
	 * Should be called when playback is finished and the data is no longer needed.
	 */
	virtual void ReleaseData() override
	{
		RemoveFromRoot();
	}
#pragma endregion

#pragma region public API
public:
	/**
	 * Constructor for the Audio2Face lipsync data holder.
	 * Adds this object to the root set to prevent garbage collection during playback.
	 */
	ULipSyncDataA2F() : ILipSyncBaseData()
	{
		AddToRoot();
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
		m_curveWeights = sourceCurves;
		m_framesPerSecond = framesPerSecond;
	}

	/** @return The FPS that the curves were generated at. */
	int GetFramePerSecond() const
	{
		return m_framesPerSecond;
	}

	/** @return A direct reference to the generated curve weights. */
	const TArray<TArray<float>>& GetA2FCurveWeights() const
	{
		return m_curveWeights;
	}
#pragma endregion

#pragma region data
private:
	int m_framesPerSecond = 0;
	TArray<TArray<float>> m_curveWeights;
#pragma endregion
};
