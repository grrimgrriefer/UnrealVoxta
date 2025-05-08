// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Audio2FacePlaybackHandler.generated.h"

class UAudioComponent;
class ULipSyncDataA2F;

/**
 * UAudio2FacePlaybackHandler
 * Handles synchronization and playback of Audio2Face lipsync data with audio.
 * Each AI Character using A2F has one instance that manages its playback.
 * Provides methods to initialize with an audio component, play/stop A2F lipsync data,
 * and retrieve current curve weights for ARKit blendshapes.
 */
UCLASS()
class VOXTAUTILITY_A2F_API UAudio2FacePlaybackHandler : public UObject
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Register the AudioComponent that will be used to sync the A2F lipsync data.
	 *
	 * @param audioComponent The component to sync with A2F lipsync data.
	 */
	void Initialize(UAudioComponent* audioComponent);

	/**
	 * Fetch the curve values for the upcoming frame, mapping to the blendshapes of the ARKit.
	 * Should only be called in pre-update via the Animator.
	 *
	 * @param targetArrayRef The array to populate with new curve values.
	 */
	void GetA2FCurveWeights(TArray<float>& targetArrayRef);

	/**
	 * Begin playback of the A2F lipsync data along with the audio in the AudioComponent.
	 * The correct soundwave must be assigned & loaded into the AudioComponent before calling this.
	 *
	 * @param lipsyncData The A2F data matching the SoundWave currently assigned to the AudioComponent.
	 */
	void Play(const ULipSyncDataA2F* lipsyncData);

	/**
	 * Stop the playback and return to a lipsync state (closed mouth).
	 */
	void Stop();
#pragma endregion

#pragma region UObject overrides
	/**
	 * Called when the object is being destroyed.
	 * Ensures delegates are properly unbound before destruction.
	 */
	virtual void BeginDestroy() override;
#pragma endregion

#pragma region data
public:
	/** Number of ARKit blendshape curves. */
	static const int CURVE_COUNT = 52;
	/** Names of the ARKit blendshape curves. */
	static const FName CURVE_NAMES[CURVE_COUNT];

private:
	UPROPERTY()
	const ULipSyncDataA2F* m_lipsyncData = nullptr;

	UPROPERTY()
	UAudioComponent* m_audioComponent = nullptr;

	FCriticalSection m_curvesGuard;

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
	 * Applies interpolation to avoid framerate issues (A2F is rendered at 30fps).
	 *
	 * @param audioComponent The component currently playing the audio.
	 * @param soundWave The soundwave currently being played.
	 * @param Percent How far along we are in the audio (normalized).
	 */
	void OnAudioPlaybackPercent(const UAudioComponent* audioComponent, const USoundWave* soundWave, float Percent);

	/**
	 * Triggered by the UAudioComponent, marks the audio playback as finished & applies the neutral pose again.
	 *
	 * @param audioComponent The component that has finished playing the audio.
	 */
	void OnAudioPlaybackFinished(UAudioComponent* audioComponent);
#pragma endregion
};
