// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAudioPlayback.h"

UVoxtaAudioPlayback::UVoxtaAudioPlayback()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVoxtaAudioPlayback::InitializeAudioPlayback(UVoxtaClient* voxtaClient, FStringView characterId)
{
	m_characterId = characterId;
	m_clientReference = voxtaClient;
	m_clientReference->VoxtaClientCharMessageAddedEvent.AddUniqueDynamic(this, &UVoxtaAudioPlayback::PlaybackMessage);
	m_hostAddress = voxtaClient->GetServerAddress();
	m_hostPort = voxtaClient->GetServerPort();
}

void UVoxtaAudioPlayback::BeginPlay()
{
	Super::BeginPlay();
	OnAudioFinished.AddUniqueDynamic(this, &UVoxtaAudioPlayback::OnAudioPlaybackFinished);
#if WITH_OVRLIPSYNC
	m_ovrLipSync = GetOwner()->FindComponentByClass<UOVRLipSyncPlaybackActorComponent>();
#endif
}

void UVoxtaAudioPlayback::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	OnAudioFinished.RemoveDynamic(this, &UVoxtaAudioPlayback::OnAudioPlaybackFinished);
	if (m_clientReference)
	{
		m_clientReference->VoxtaClientCharMessageAddedEvent.RemoveDynamic(this, &UVoxtaAudioPlayback::PlaybackMessage);
	}
}

bool UVoxtaAudioPlayback::IsPlaying() const
{
	return isPlaying;
}

void UVoxtaAudioPlayback::PlaybackMessage(const FCharDataBase& sender, const FChatMessage& message)
{
	if (sender.GetId() == m_characterId)
	{
		Cleanup();
		m_messageId = message.GetMessageId();

		for (int i = 0; i < message.m_audioUrls.Num(); i++)
		{
			m_orderedAudio.Add(MessageChunkAudioContainer(
				FString::Format(*FString(TEXT("http://{0}:{1}{2}")), { m_hostAddress, m_hostPort, message.m_audioUrls[i] }),
				m_lipSyncType,
				[&] (const MessageChunkAudioContainer* chunk) { OnChunkStateChange(chunk); },
				i));
		}
		m_orderedAudio[0].Continue();
	}
}

void UVoxtaAudioPlayback::TryPlayCurrentAudioChunk()
{
	if (m_orderedAudio[currentAudioClip].m_state == MessageChunkState::ReadyForPlayback)
	{
		SetSound(m_orderedAudio[currentAudioClip].m_soundWave);

		// TODO: move this to somewhere else if things get too complicated
		switch (m_orderedAudio[currentAudioClip].m_lipSyncType)
		{
			case LipSyncType::None:
				Play();
				break;
			case LipSyncType::Custom:
				UE_LOG(LogTemp, Error, TEXT("TODO: add blueprint support for this"));
				break;
			case LipSyncType::OVRLipSync:
#if WITH_OVRLIPSYNC
				m_ovrLipSync->Start(this, m_orderedAudio[currentAudioClip].m_lipSyncData.GetOvrLipSyncData());
#else
				UE_LOG(LogTemp, Error, TEXT("OvrLipSync was selected, but the module is not present in the project."));
#endif
				break;
			default:
				UE_LOG(LogTemp, Error, TEXT("Unsupported selection for LipSync, this should never happen."));
				break;
		}

		isPlaying = true;
	}
}

void UVoxtaAudioPlayback::OnAudioPlaybackFinished()
{
	isPlaying = false;
	currentAudioClip += 1;
	if (currentAudioClip < m_orderedAudio.Num())
	{
		TryPlayCurrentAudioChunk();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Playback finished."));
		VoxtaMessageAudioPlaybackEvent.Broadcast(m_messageId);
		Cleanup();
	}
}

void UVoxtaAudioPlayback::OnChunkStateChange(const MessageChunkAudioContainer* chunk)
{
	if (m_orderedAudio[chunk->m_id].m_state == MessageChunkState::ReadyForPlayback && !isPlaying)
	{
		TryPlayCurrentAudioChunk();
	}
	m_orderedAudio[chunk->m_id].Continue();
	if (chunk->m_id + 1 < m_orderedAudio.Num())
	{
		m_orderedAudio[chunk->m_id + 1].Continue();
	}
}

void UVoxtaAudioPlayback::Cleanup()
{
	m_messageId.Empty();
	currentAudioClip = 0;
	isPlaying = false;
	for (MessageChunkAudioContainer& audioChunk : m_orderedAudio)
	{
		audioChunk.CleanupData();
	}
	m_orderedAudio.Empty();
}