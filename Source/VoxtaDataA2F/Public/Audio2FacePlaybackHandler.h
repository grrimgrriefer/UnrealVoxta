// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "LipSyncDataA2F.h"
#include "Audio2FacePlaybackHandler.generated.h"

UCLASS()
class VOXTADATAA2F_API UAudio2FacePlaybackHandler : public UObject
{
	GENERATED_BODY()

public:
	static const FName CurveNames[52];

	UAudio2FacePlaybackHandler();

	void GetA2FCurveWeights(TArray<float>& sourceCurves) const;
	void Play(UAudioComponent* InAudioComponent, const ULipSyncDataA2F* InSequence);
	void Stop();

protected:
	void OnAudioPlaybackPercent(const UAudioComponent*, const USoundWave*, float Percent);
	void OnAudioPlaybackFinished(UAudioComponent*);

private:
	const ULipSyncDataA2F* m_lipsyncData;
	UAudioComponent* m_audioComponent;

	FDelegateHandle PlaybackPercentHandle;
	FDelegateHandle PlaybackFinishedHandle;

	TArray<float> m_currentCurves;

	bool m_forcedNeutral = true;

	void InitNeutralPose();
};
