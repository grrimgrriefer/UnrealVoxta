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

	UVoxtaGlobalAudioPlayback* GetGlobalPlaybackComponent() const;
#pragma endregion

#pragma region data
private:
	UVoxtaGlobalAudioPlayback* m_globalAudioPlayback;
};
