// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAudioPlayback.h"

UVoxtaAudioPlayback::UVoxtaAudioPlayback()
{
	m_audioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	//	m_audioComponent->SetupAttachment(GetOwner()->GetRootComponent());
	PrimaryComponentTick.bCanEverTick = true;

	ConstructorHelpers::FObjectFinder<USoundCue> propellerCue(TEXT("/Game/SavedFile_Cue"));
	propellerAudioCue = propellerCue.Object;
}

void UVoxtaAudioPlayback::InitializeAudioPlayback(UVoxtaClient* voxtaClient, FStringView characterId)
{
	m_characterId = characterId;
	voxtaClient->VoxtaClientCharMessageAddedEvent.AddUniqueDynamic(this, &UVoxtaAudioPlayback::PlaybackMessage);
	m_hostAddress = voxtaClient->GetServerAddress();
	m_hostPort = voxtaClient->GetServerPort();
}

void UVoxtaAudioPlayback::PlaybackMessage(const FCharDataBase& sender, const FChatMessage& message)
{
	if (sender.GetId() == m_characterId)
	{
		m_orderedUrls.Empty();
		m_audioData.Empty();
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
		USoundWaveProcedural soundWaveProcedural = ConvertRawAudioData(response->GetContent().GetData());
		m_audioData.Emplace(request.Get()->GetURL(), soundWaveProcedural);

		if (m_audioData.Num() == m_orderedUrls.Num())
		{
			TriggerPlayback();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("File download failed."));
	}
}

void UVoxtaAudioPlayback::TriggerPlayback()
{
	//m_audioComponent->SetSound(m_audioData[m_orderedUrls[0]]);
	m_audioComponent->SetSound(propellerAudioCue);
	m_audioComponent->Play();

	UE_LOG(LogTemp, Warning, TEXT("Playing audio."));
}

USoundWaveProcedural UVoxtaAudioPlayback::ConvertRawAudioData(const TArray<uint8>& rawData)
{
	/*	RuntimeAudioImporter->OnProgressNative.AddWeakLambda(this, [] (int32 Percentage)
		{
			UE_LOG(LogTemp, Log, TEXT("Audio importing percentage: %d"), Percentage);
		});

		RuntimeAudioImporter->OnResultNative.AddWeakLambda(this, [this] (URuntimeAudioImporterLibrary* Importer, UImportedSoundWave* ImportedSoundWave, ERuntimeImportStatus Status)
		{
			if (Status == ERuntimeImportStatus::SuccessfulImport)
			{
				UE_LOG(LogTemp, Warning, TEXT("Successfully imported audio with sound wave %s"), *ImportedSoundWave->GetName());
				// Here you can handle ImportedSoundWave playback, like "UGameplayStatics::PlaySound2D(GetWorld(), ImportedSoundWave);"
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to import audio"));
			}

			RuntimeAudioImporter = nullptr;
		});*/

	audioImporter.ImportAudioFromBuffer(TArray64<uint8>(rawData));
}