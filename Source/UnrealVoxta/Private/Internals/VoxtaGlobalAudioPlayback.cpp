// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaGlobalAudioPlayback.h"
#include "BaseCharData.h"
#include "VoxtaClient.h"
#include "Logging/StructuredLog.h"

void UVoxtaGlobalAudioPlayback::SetEnabled(bool newState)
{
	m_isEnabled = newState;	
}

bool UVoxtaGlobalAudioPlayback::IsEnabled() const
{
	return m_isEnabled;
}

void UVoxtaGlobalAudioPlayback::PlaybackMessage(const FBaseCharData& sender, const FChatMessage& message)
{
	if (m_isEnabled)
	{
		if (!m_isInitialized)
		{
			Prepare();
		}

		TGuardValue<FGuid> GuardCharacterId(m_characterId, sender.GetId());
		UVoxtaAudioPlayback::PlaybackMessage(sender, message);
	}	
}

void UVoxtaGlobalAudioPlayback::Prepare()
{
	USoundAttenuation* settings = NewObject<USoundAttenuation>(this);
	if (!settings)
	{
		UE_LOGFMT(VoxtaLog, Error, "Failed to create USoundAttenuation for global audio playback");
		return;
	}

	settings->Attenuation.bAttenuate = false;
	settings->Attenuation.bSpatialize = false;
	SetAttenuationSettings(MoveTemp(settings));
	
	InitializeInternal(false);
	if (!IsValid(m_clientReference))
	{
		UE_LOGFMT(VoxtaLog, Error, "Failed to initialize global audio playback - invalid client reference");
		return;
	}

	m_characterId = FGuid();
	m_isInitialized = true;
}