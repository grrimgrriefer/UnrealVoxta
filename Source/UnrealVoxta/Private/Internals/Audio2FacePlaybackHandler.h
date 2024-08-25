// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "LipSyncDataA2F.h"
#include "TimedWeightSample.h"

class Audio2FacePlaybackHandler
{
public:
	Audio2FacePlaybackHandler();

	void GetA2FCurveWeights(TArray<float>& sourceCurves);
	void Play(UAudioComponent* InAudioComponent, ULipSyncDataA2F* InSequence);

	static const FName CurveNames[55];

private:
	ULipSyncDataA2F* m_lipsyncData;
	UAudioComponent* m_audioComponent;
};
