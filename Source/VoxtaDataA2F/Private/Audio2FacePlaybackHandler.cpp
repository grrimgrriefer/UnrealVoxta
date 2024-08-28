﻿// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "Audio2FacePlaybackHandler.h"
#include "VoxtaDefines.h"
#include <Logging/StructuredLog.h>

// ArkitNames https://developer.apple.com/documentation/arkit/arfaceanchor/blendshapelocation
const FName UAudio2FacePlaybackHandler::CurveNames[52] =
{
	API_NAME("EyeBlinkLeft"),
	API_NAME("EyeLookDownLeft"),
	API_NAME("EyeLookInLeft"),
	API_NAME("EyeLookOutLeft"),
	API_NAME("EyeLookUpLeft"),
	API_NAME("EyeSquintLeft"),
	API_NAME("EyeWideLeft"),
	API_NAME("EyeBlinkRight"),
	API_NAME("EyeLookDownRight"),
	API_NAME("EyeLookInRight"),
	API_NAME("EyeLookOutRight"),
	API_NAME("EyeLookUpRight"),
	API_NAME("EyeSquintRight"),
	API_NAME("EyeWideRight"),
	API_NAME("JawForward"),
	API_NAME("JawLeft"),
	API_NAME("JawRight"),
	API_NAME("JawOpen"),
	API_NAME("MouthClose"),
	API_NAME("MouthFunnel"),
	API_NAME("MouthPucker"),
	API_NAME("MouthLeft"),
	API_NAME("MouthRight"),
	API_NAME("MouthSmileLeft"),
	API_NAME("MouthSmileRight"),
	API_NAME("MouthFrownLeft"),
	API_NAME("MouthFrownRight"),
	API_NAME("MouthDimpleLeft"),
	API_NAME("MouthDimpleRight"),
	API_NAME("MouthStretchLeft"),
	API_NAME("MouthStretchRight"),
	API_NAME("MouthRollLower"),
	API_NAME("MouthRollUpper"),
	API_NAME("MouthShrugLower"),
	API_NAME("MouthShrugUpper"),
	API_NAME("MouthPressLeft"),
	API_NAME("MouthPressRight"),
	API_NAME("MouthLowerDownLeft"),
	API_NAME("MouthLowerDownRight"),
	API_NAME("MouthUpperUpLeft"),
	API_NAME("MouthUpperUpRight"),
	API_NAME("BrowDownLeft"),
	API_NAME("BrowDownRight"),
	API_NAME("BrowInnerUp"),
	API_NAME("BrowOuterUpLeft"),
	API_NAME("BrowOuterUpRight"),
	API_NAME("CheekPuff"),
	API_NAME("CheekSquintLeft"),
	API_NAME("CheekSquintRight"),
	API_NAME("NoseSneerLeft"),
	API_NAME("NoseSneerRight"),
	API_NAME("TongueOut")
};

UAudio2FacePlaybackHandler::UAudio2FacePlaybackHandler()
{
}

void UAudio2FacePlaybackHandler::GetA2FCurveWeights(TArray<float>& targetArrayRef) const
{
	if (m_forcedNeutral)
	{
		targetArrayRef.Empty(52);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Fetching current curves"));
		targetArrayRef = m_currentCurves;
	}
}

void UAudio2FacePlaybackHandler::Play(UAudioComponent* audioComponent, ULipSyncDataA2F* lipsyncData)
{
	m_audioComponent = audioComponent;
	m_lipsyncData = lipsyncData;

	m_forcedNeutral = false;

	PlaybackPercentHandle = m_audioComponent->OnAudioPlaybackPercentNative.AddUObject(
		this, &UAudio2FacePlaybackHandler::OnAudioPlaybackPercent);
	PlaybackFinishedHandle = m_audioComponent->OnAudioFinishedNative.AddUObject(
		this, &UAudio2FacePlaybackHandler::OnAudioPlaybackFinished);
	m_audioComponent->Play();
}

void UAudio2FacePlaybackHandler::Stop()
{
	if (!m_audioComponent)
	{
		return;
	}
	m_audioComponent->OnAudioPlaybackPercentNative.Remove(PlaybackPercentHandle);
	m_audioComponent->OnAudioFinishedNative.Remove(PlaybackFinishedHandle);
	m_audioComponent = nullptr;
	InitNeutralPose();
}

void UAudio2FacePlaybackHandler::OnAudioPlaybackPercent(const UAudioComponent*, const USoundWave* SoundWave, float Percent)
{
	if (m_lipsyncData == nullptr)
	{
		InitNeutralPose();
		return;
	}
	UE_LOGFMT(LogTemp, Log, "percentage: {0}", Percent);
	float currentFrame = SoundWave->Duration * m_lipsyncData->GetFramePerSecond() * Percent;
	float totalFrameCount = m_lipsyncData->GetA2FCurveWeights().Num();
	int closestFrame = FMath::RoundToInt(currentFrame);
	int floorFrame = FMath::Floor(currentFrame);
	if (closestFrame >= totalFrameCount)
	{
		InitNeutralPose();
		return;
	}
	if (FMath::IsNearlyEqual(currentFrame, closestFrame))
	{
		m_currentCurves = m_lipsyncData->GetA2FCurveWeights()[closestFrame];
		return;
	}
	else if (floorFrame + 1 < totalFrameCount)
	{
		float normalizedBlend = currentFrame - floorFrame;
		const TArray<float>& floor = m_lipsyncData->GetA2FCurveWeights()[floorFrame];
		const TArray<float>& ceiling = m_lipsyncData->GetA2FCurveWeights()[floorFrame + 1];

		for (int i = 0; i < floor.Num(); i++)
		{
			m_currentCurves[i] = (floor[i] * normalizedBlend) + (ceiling[i] * (1.f - normalizedBlend));
		}
	}
	else
	{
		// Error too few lipsync frames, just pick last values
		m_currentCurves = m_lipsyncData->GetA2FCurveWeights()[totalFrameCount - 1];
	}
}

void UAudio2FacePlaybackHandler::OnAudioPlaybackFinished(UAudioComponent*)
{
	InitNeutralPose();
}

void UAudio2FacePlaybackHandler::InitNeutralPose()
{
	m_forcedNeutral = true;
}