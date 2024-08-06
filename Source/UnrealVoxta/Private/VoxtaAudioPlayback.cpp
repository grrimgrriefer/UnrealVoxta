// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAudioPlayback.h"

UVoxtaAudioPlayback::UVoxtaAudioPlayback()
{
	m_audioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	PrimaryComponentTick.bCanEverTick = true;
}

void UVoxtaAudioPlayback::BeginPlay()
{
	Super::BeginPlay();
	m_audioComponent->OnAudioFinished.AddUniqueDynamic(this, &UVoxtaAudioPlayback::OnAudioFinished);
}

void UVoxtaAudioPlayback::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (m_audioComponent)
	{
		m_audioComponent->OnAudioFinished.RemoveDynamic(this, &UVoxtaAudioPlayback::OnAudioFinished);
	}
	if (m_clientReference)
	{
		m_clientReference->VoxtaClientCharMessageAddedEvent.RemoveDynamic(this, &UVoxtaAudioPlayback::PlaybackMessage);
	}
}

void UVoxtaAudioPlayback::InitializeAudioPlayback(UVoxtaClient* voxtaClient, FStringView characterId)
{
	m_characterId = characterId;
	m_clientReference = voxtaClient;
	m_clientReference->VoxtaClientCharMessageAddedEvent.AddUniqueDynamic(this, &UVoxtaAudioPlayback::PlaybackMessage);
	m_hostAddress = voxtaClient->GetServerAddress();
	m_hostPort = voxtaClient->GetServerPort();
}

bool UVoxtaAudioPlayback::IsPlaying() const
{
	return isPlaying;
}

void UVoxtaAudioPlayback::PlaybackMessage(const FCharDataBase& sender, const FChatMessage& message)
{
	if (sender.GetId() == m_characterId)
	{
		currentAudioClip = 0;
		isPlaying = false;
		for (MessageChunkAudioContainer& audioChunk : m_orderedAudio)
		{
			audioChunk.Cleanup();
		}
		m_orderedAudio.Empty();
		m_messageId = message.GetMessageId();

		for (int i = 0; i < message.m_audioUrls.Num(); i++)
		{
			m_orderedAudio.Add(MessageChunkAudioContainer(FString::Format(*FString(TEXT("http://{0}:{1}{2}")), {
				m_hostAddress,
				m_hostPort,
				message.m_audioUrls[i] })));
		}

		m_orderedAudio[0].DownloadAsync();
	}
}

void UVoxtaAudioPlayback::TryPlayNextAudio()
{
}

void UVoxtaAudioPlayback::OnAudioFinished()
{
	isPlaying = false;
	TryPlayNextAudio();
}