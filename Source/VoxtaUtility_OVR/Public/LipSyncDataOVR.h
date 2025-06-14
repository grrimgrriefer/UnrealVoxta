// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncBaseData.h"
#include "LipSyncDataOVR.generated.h"

class UOVRLipSyncFrameSequence;

/**
 * ULipSyncDataOVR
 * Contains all the data required for playback of OVR lipsync generation.
 * Used to keep OVR lipsync logic modular and separated from other lipsync types.
 */
UCLASS(Category = "Voxta")
class VOXTAUTILITY_OVR_API ULipSyncDataOVR : public UObject, public ILipSyncBaseData
{
	GENERATED_BODY()

#pragma region ILipSyncBaseData overrides
public:
	/**
	 * Clean up the OVR-lipsync data that was made & kept in memory for playback of the voiceline that is tied
	 * to this data structure.
	 */
	virtual void ReleaseData() override
	{
		m_ovrLipSyncFrameSequence = nullptr;
		RemoveFromRoot();
	}
#pragma endregion

#pragma region public API
public:
	/**
	 * Constructor for the ULipSyncDataOVR lipsync data holder.
	 * Adds this object to the root set to prevent garbage collection during playback.
	 */
	ULipSyncDataOVR() : ILipSyncBaseData()
	{
		AddToRoot();
	}

	/**
	 * Store the generated UOVRLipSyncFrameSequence in this instance.
	 * The sequence will remain valid as long as this ULipSyncDataOVR object exists.
	 *
	 * @param ovrLipSyncFrameSequence The sequence of OVR curves that will be provided to the OVR audio playback
	 * component when playing the matching audio.
	 */
	void SetFrameSequence(UOVRLipSyncFrameSequence* ovrLipSyncFrameSequence)
	{
		m_ovrLipSyncFrameSequence = ovrLipSyncFrameSequence;
	}

	/**
	 * Get the OVR lipsync frame sequence data.
	 * 
	 * @return Pointer to the OVR lipsync frame sequence.
	 */
	UOVRLipSyncFrameSequence* GetOvrLipSyncData() const
	{
		return m_ovrLipSyncFrameSequence;
	}
#pragma endregion

#pragma region data
private:
	UPROPERTY()
	UOVRLipSyncFrameSequence* m_ovrLipSyncFrameSequence = nullptr;
#pragma endregion
};
