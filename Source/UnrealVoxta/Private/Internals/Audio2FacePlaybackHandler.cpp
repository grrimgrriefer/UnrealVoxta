// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "Audio2FacePlaybackHandler.h"
#include "VoxtaDefines.h"

// ArkitNames https://developer.apple.com/documentation/arkit/arfaceanchor/blendshapelocation
const FName Audio2FacePlaybackHandler::CurveNames[55] =
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

Audio2FacePlaybackHandler::Audio2FacePlaybackHandler()
{
}

void Audio2FacePlaybackHandler::GetA2FCurveWeights(TArray<float>& targetArrayRef)
{
	TArray<TimedWeightSample> m_curveSamples = m_lipsyncData->GetA2FCurveWeights();
	m_curveSamples();
}

void Audio2FacePlaybackHandler::Play(UAudioComponent* audioComponent, ULipSyncDataA2F* lipsyncData)
{
	m_audioComponent = audioComponent;
	m_lipsyncData = lipsyncData;
}