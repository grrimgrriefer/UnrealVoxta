// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "Audio2FacePlaybackHandler.h"
#include "VoxtaDefines.h"
#include <Logging/StructuredLog.h>

// ArkitNames https://developer.apple.com/documentation/arkit/arfaceanchor/blendshapelocation
const FName UAudio2FacePlaybackHandler::CurveNames[52] =
{
	EASY_NAME("EyeBlinkLeft"),
	EASY_NAME("EyeLookDownLeft"),
	EASY_NAME("EyeLookInLeft"),
	EASY_NAME("EyeLookOutLeft"),
	EASY_NAME("EyeLookUpLeft"),
	EASY_NAME("EyeSquintLeft"),
	EASY_NAME("EyeWideLeft"),
	EASY_NAME("EyeBlinkRight"),
	EASY_NAME("EyeLookDownRight"),
	EASY_NAME("EyeLookInRight"),
	EASY_NAME("EyeLookOutRight"),
	EASY_NAME("EyeLookUpRight"),
	EASY_NAME("EyeSquintRight"),
	EASY_NAME("EyeWideRight"),
	EASY_NAME("JawForward"),
	EASY_NAME("JawLeft"),
	EASY_NAME("JawRight"),
	EASY_NAME("JawOpen"),
	EASY_NAME("MouthClose"),
	EASY_NAME("MouthFunnel"),
	EASY_NAME("MouthPucker"),
	EASY_NAME("MouthLeft"),
	EASY_NAME("MouthRight"),
	EASY_NAME("MouthSmileLeft"),
	EASY_NAME("MouthSmileRight"),
	EASY_NAME("MouthFrownLeft"),
	EASY_NAME("MouthFrownRight"),
	EASY_NAME("MouthDimpleLeft"),
	EASY_NAME("MouthDimpleRight"),
	EASY_NAME("MouthStretchLeft"),
	EASY_NAME("MouthStretchRight"),
	EASY_NAME("MouthRollLower"),
	EASY_NAME("MouthRollUpper"),
	EASY_NAME("MouthShrugLower"),
	EASY_NAME("MouthShrugUpper"),
	EASY_NAME("MouthPressLeft"),
	EASY_NAME("MouthPressRight"),
	EASY_NAME("MouthLowerDownLeft"),
	EASY_NAME("MouthLowerDownRight"),
	EASY_NAME("MouthUpperUpLeft"),
	EASY_NAME("MouthUpperUpRight"),
	EASY_NAME("BrowDownLeft"),
	EASY_NAME("BrowDownRight"),
	EASY_NAME("BrowInnerUp"),
	EASY_NAME("BrowOuterUpLeft"),
	EASY_NAME("BrowOuterUpRight"),
	EASY_NAME("CheekPuff"),
	EASY_NAME("CheekSquintLeft"),
	EASY_NAME("CheekSquintRight"),
	EASY_NAME("NoseSneerLeft"),
	EASY_NAME("NoseSneerRight"),
	EASY_NAME("TongueOut")
};

void UAudio2FacePlaybackHandler::Initialize(UAudioComponent* audioComponent)
{
	m_audioComponent = audioComponent;
}

void UAudio2FacePlaybackHandler::GetA2FCurveWeights(TArray<float>& targetArrayRef) const
{
	if (m_forcedNeutral)
	{
		targetArrayRef.Empty(52);
	}
	else
	{
		targetArrayRef = m_currentCurves;
	}
}

void UAudio2FacePlaybackHandler::Play(const ULipSyncDataA2F* lipsyncData)
{
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
	float currentFrame = SoundWave->Duration * m_lipsyncData->GetFramePerSecond() * Percent;
	float totalFrameCount = m_lipsyncData->GetA2FCurveWeights().Num();
	int closestFrame = FMath::RoundToInt(currentFrame);
	if (closestFrame >= totalFrameCount)
	{
		InitNeutralPose();
		// this should never happen
		return;
	}
	if (FMath::IsNearlyEqual(currentFrame, closestFrame))
	{
		m_currentCurves = m_lipsyncData->GetA2FCurveWeights()[closestFrame];
		return;
	}
	int ceilingFrame = FMath::CeilToInt(currentFrame);
	int floorFrame = FMath::FloorToInt(currentFrame);
	if (ceilingFrame < totalFrameCount)
	{
		float normalizedBlend = currentFrame - floorFrame;
		const TArray<float>& floor = m_lipsyncData->GetA2FCurveWeights()[floorFrame];
		const TArray<float>& ceiling = m_lipsyncData->GetA2FCurveWeights()[ceilingFrame];

		for (int i = 0; i < floor.Num(); i++)
		{
			m_currentCurves[i] = (floor[i] * (1.f - normalizedBlend)) + (ceiling[i] * normalizedBlend);
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