// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "VoxtaAudioPlayback.h"
#include "VoxtaGlobalAudioPlayback.generated.h"

UCLASS(ClassGroup = Voxta, NotPlaceable, NotBlueprintType)
class UVoxtaGlobalAudioPlayback : public UVoxtaAudioPlayback
{
	GENERATED_BODY()

#pragma region public API
public:
	void SetEnabled(bool newState);

	bool IsEnabled() const;

	virtual void PlaybackMessage(const FBaseCharData& sender, const FChatMessage& message) override;
#pragma endregion

#pragma region lipsync related API
private:
	using UVoxtaAudioPlayback::Initialize;
	using UVoxtaAudioPlayback::MarkAudioChunkCustomPlaybackComplete;
	using UVoxtaAudioPlayback::GetLipSyncType;
	using UVoxtaAudioPlayback::GetA2FCurveWeightsPreUpdate;
#pragma endregion

#pragma region data
private:
	LipSyncType m_lipSyncType = LipSyncType::None;

	bool m_isEnabled = false;
	bool m_isInitialized = false;
#pragma endregion

#pragma region private API
private:
	void Prepare();
#pragma endregion
};
