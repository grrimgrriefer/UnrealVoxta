// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "LipSyncBaseData.h"
#include "LipSyncDataOVR.generated.h"

class UOVRLipSyncFrameSequence;

/**
 * ULipSyncDataOVR.
 * Wrapper for OVR-lipsync specific data. This lives in its own module, which is excluded from the others if
 * OVR plugin is not installed in the project. (i.e. if the dev doesn't want lipsync for some reason)
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
	virtual void CleanupData() override
	{
		m_ovrLipSyncFrameSequence = nullptr;
		this->RemoveFromRoot();
	}
#pragma endregion

#pragma region public API
public:
	/** Create an instance of the LipSyncData holder for OVRLipSync. */
	ULipSyncDataOVR() : ILipSyncBaseData(LipSyncType::OVRLipSync)
	{
	}

	/**
	 * Register the generated UOVRLipSyncFrameSequence as part of this instance..
	 * This will transfer ownership of the provided pointer to this object.
	 *
	 * @param ovrLipSyncFrameSequence The sequence of OVR curves that will be provided to the OVR audio playback
	 * component when playing the matching audio.
	 */
	void SetFrameSequence(UOVRLipSyncFrameSequence* ovrLipSyncFrameSequence)
	{
		m_ovrLipSyncFrameSequence = MoveTemp(ovrLipSyncFrameSequence);
	}

	/** @return A raw pointer to the OVR curves that were generated and assigned to this data instance */
	UOVRLipSyncFrameSequence* GetOvrLipSyncData() const
	{
		return m_ovrLipSyncFrameSequence;
	}
#pragma endregion

#pragma region data
private:
	UPROPERTY()
	UOVRLipSyncFrameSequence* m_ovrLipSyncFrameSequence;
#pragma endregion
};
