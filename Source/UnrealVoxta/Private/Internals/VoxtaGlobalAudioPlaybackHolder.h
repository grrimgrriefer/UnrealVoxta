// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "VoxtaGlobalAudioPlaybackHolder.generated.h"

class UVoxtaGlobalAudioPlayback;

UCLASS(ClassGroup = Voxta, NotPlaceable, NotBlueprintType)
class AVoxtaGlobalAudioPlaybackHolder : public AActor
{
	GENERATED_BODY()

#pragma region public API
public:
	AVoxtaGlobalAudioPlaybackHolder();

	/** @return The global audio playback component that handles playback when no character-specific handler is available */
	UVoxtaGlobalAudioPlayback* GetGlobalPlaybackComponent() const;
#pragma endregion

#pragma region data
private:
	UPROPERTY()
	UVoxtaGlobalAudioPlayback* m_globalAudioPlayback;
};
