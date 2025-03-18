// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaGlobalAudioPlayback.h"
#include "BaseCharData.h"

void UVoxtaGlobalAudioPlayback::SetEnabled(bool newState)
{
	m_isEnabled = newState;	
}

void UVoxtaGlobalAudioPlayback::PlaybackMessage(const FBaseCharData& sender, const FChatMessage& message)
{
	if (m_isEnabled)
	{
		if (!m_isInitialized)
		{
			Prepare();
		}

		m_characterId = sender.GetId();
		UVoxtaAudioPlayback::PlaybackMessage(sender, message);
		m_characterId = FGuid();
	}	
}

void UVoxtaGlobalAudioPlayback::Prepare()
{
	USoundAttenuation* settings = NewObject<USoundAttenuation>();
	settings->Attenuation.bAttenuate = false;
	settings->Attenuation.bSpatialize = false;
	SetAttenuationSettings(MoveTemp(settings));
	InitializeInternal(false);

	m_characterId = FGuid();
	m_isInitialized = true;
}