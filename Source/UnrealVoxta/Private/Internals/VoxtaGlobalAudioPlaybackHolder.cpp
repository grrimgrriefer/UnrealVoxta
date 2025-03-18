// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "VoxtaGlobalAudioPlayback.h"
#include "VoxtaGlobalAudioPlaybackHolder.h"

AVoxtaGlobalAudioPlaybackHolder::AVoxtaGlobalAudioPlaybackHolder()
{
	PrimaryActorTick.bCanEverTick = false;
	m_globalAudioPlayback = CreateDefaultSubobject<UVoxtaGlobalAudioPlayback>(TEXT("GlobalAudioPlayback"));
}

UVoxtaGlobalAudioPlayback* AVoxtaGlobalAudioPlaybackHolder::GetGlobalPlaybackComponent() const
{
	return m_globalAudioPlayback;
}