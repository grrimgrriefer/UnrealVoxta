// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Audio2FacePlaybackHandler.generated.h"

class UAudioComponent;
class ULipSyncDataA2F;

/**
 * UAudio2FacePlaybackHandler.
 * Ensures that the A2F data is played in sync with the AudioComponent, regardless of the framerate.
 * Each Ai Character that uses A2F has one instance of this, that manges just its own playback.
 */
UCLASS()
class VOXTAUTILITY_A2F_API UAudio2FacePlaybackHandler : public UObject
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Register the AudioComponent that we will use to sync the A2F lipsync data with.
	 *
	 * @param audioComponent The component that we will use to sync the A2F lipsync data with.
	 */
	void Initialize(UAudioComponent* audioComponent);

	/**
	 * Fetch the curve values for upcoming frame, mapping to the blendshapes of the ARKit.
	 * Note: This should only be called in pre-update via the Animator.
	 *
	 * @param targetArrayRef The array that will be populated with the new curve values.
	 */
	void GetA2FCurveWeights(TArray<float>& targetArrayRef) const;

	/**
	 * Begin playback of the A2F lipsync data along with the audio in the AudioComponent.
	 *
	 * Note: It is expected that the correct soundwave is assigned & loaded into the AudioComponent BEFORE
	 * attempting to play the A2F data.
	 *
	 * @param lipsyncData The A2F data that matches the SoundWave currently assigned to the AudioComponent.
	 */
	void Play(const ULipSyncDataA2F* lipsyncData);

	/** Stop the playback and return back to a lipsync state (closed mouth). */
	void Stop();
#pragma endregion

#pragma region data
public:
	static const FName CURVE_NAMES[52];

private:
	const ULipSyncDataA2F* m_lipsyncData;
	UAudioComponent* m_audioComponent;

	FDelegateHandle m_playbackPercentHandle;
	FDelegateHandle m_playbackFinishedHandle;
	TArray<float> m_currentCurves;
	bool m_forcedNeutral = true;
#pragma endregion

#pragma region private API
	/** Trigger a neutral pose for all GetA2FCurveWeights until a new Play has been called with new data. */
	void InitNeutralPose();

	/**
	 * Triggered by the UAudioComponent, updates the currentCurve data to represent the current state of the audio.
	 * Will also apply interpolation to avoid framerate issues (A2F is rendered at 30fps)
	 *
	 * @param audioComponent The component currently playing the audio.
	 * @param soundWave The soundwave currently being played.
	 * @param Percent How far along we are in the audio. (normalized)
	 */
	void OnAudioPlaybackPercent(const UAudioComponent* audioComponent, const USoundWave* soundWave, float Percent);

	/**
	 * Triggered by the UAudioComponent, mark the audio playback as finished & apply the neutral pose again.
	 *
	 * @param The component that has finished playing the audio.
	 */
	void OnAudioPlaybackFinished(UAudioComponent* audioComponent);
#pragma endregion
};
