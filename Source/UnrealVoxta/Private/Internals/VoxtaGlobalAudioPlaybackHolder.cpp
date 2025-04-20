// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaGlobalAudioPlaybackHolder.h"
#include "CoreMinimal.h"
#include "VoxtaGlobalAudioPlayback.h"

AVoxtaGlobalAudioPlaybackHolder::AVoxtaGlobalAudioPlaybackHolder()
{
	PrimaryActorTick.bCanEverTick = false;
	m_globalAudioPlayback = CreateDefaultSubobject<UVoxtaGlobalAudioPlayback>(TEXT("GlobalAudioPlayback"));
	SetRootComponent(m_globalAudioPlayback);
}

UVoxtaGlobalAudioPlayback* AVoxtaGlobalAudioPlaybackHolder::GetGlobalPlaybackComponent() const
{
	return m_globalAudioPlayback;
}