// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAudioPlayback.h"

UVoxtaAudioPlayback::UVoxtaAudioPlayback()
{
	m_audioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	PrimaryComponentTick.bCanEverTick = true;

	audioImporter = CreateDefaultSubobject<UAudioImporter>(TEXT("AudioImporter"));
}

void UVoxtaAudioPlayback::BeginPlay()
{
	Super::BeginPlay();
	audioImporter->AudioImportedEvent.AddUniqueDynamic(this, &UVoxtaAudioPlayback::AudioImportCompleted);
	m_audioComponent->OnAudioFinished.AddUniqueDynamic(this, &UVoxtaAudioPlayback::OnAudioFinished);
}

void UVoxtaAudioPlayback::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (audioImporter)
	{
		audioImporter->AudioImportedEvent.RemoveDynamic(this, &UVoxtaAudioPlayback::AudioImportCompleted);
	}
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
		m_orderedUrls.Empty();
		TArray<FString> keys;
		m_audioData.GetKeys(keys);
		for (int i = 0; i < keys.Num(); i++)
		{
			m_audioData[keys[i]]->RemoveFromRoot();
		}
		m_audioData.Empty();
		m_messageId = message.GetMessageId();
		GenerateFullUrls(message);
		DownloadDataAsync();
	}
}

void UVoxtaAudioPlayback::GenerateFullUrls(const FChatMessage& message)
{
	for (int i = 0; i < message.m_audioUrls.Num(); i++)
	{
		FString fullUrl = FString::Format(*FString(TEXT("http://{0}:{1}{2}")), {
			m_hostAddress,
			m_hostPort,
			message.m_audioUrls[i] });
		m_orderedUrls.Add(fullUrl);
	}
}

void UVoxtaAudioPlayback::DownloadDataAsync()
{
	for (const FString& url : m_orderedUrls)
	{
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> httpRequest = FHttpModule::Get().CreateRequest();
		httpRequest->SetVerb("GET");
		httpRequest->SetURL(url);
		httpRequest->OnProcessRequestComplete().BindUObject(this, &UVoxtaAudioPlayback::OnDownloadComplete);
		httpRequest->ProcessRequest();
	}
}

void UVoxtaAudioPlayback::OnDownloadComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful)
{
	if (bWasSuccessful && m_orderedUrls.Contains(request.Get()->GetURL()))
	{
		audioImporter->ImportAudioFromBuffer(request.Get()->GetURL(), TArray64<uint8>(response->GetContent()));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("File download failed."));
	}
}

void UVoxtaAudioPlayback::TryPlayNextAudio()
{
	if (m_orderedUrls.Num() == currentAudioClip)
	{
		UE_LOG(LogTemp, Log, TEXT("Playback finished."));
		VoxtaMessageAudioPlaybackEvent.Broadcast(m_messageId);
		m_messageId.Empty();
		return;
	}

	if (!isPlaying && m_audioData.Contains(m_orderedUrls[currentAudioClip]))
	{
		m_audioComponent->SetSound(m_audioData[m_orderedUrls[currentAudioClip]]);
		m_audioComponent->Play();
		isPlaying = true;

		currentAudioClip += 1;

		UE_LOG(LogTemp, Warning, TEXT("Playing audio."));
	}
}

void UVoxtaAudioPlayback::AudioImportCompleted(FString identifier, UImportedSoundWave* soundWave)
{
	soundWave->AddToRoot();
	m_audioData.Emplace(identifier, soundWave);
	if (!isPlaying)
	{
		TryPlayNextAudio();
	}
}

void UVoxtaAudioPlayback::OnAudioFinished()
{
	isPlaying = false;
	TryPlayNextAudio();
}