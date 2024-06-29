// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAudioPlayback.h"

UVoxtaAudioPlayback::UVoxtaAudioPlayback()
{
	m_audioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	PrimaryComponentTick.bCanEverTick = false;
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
		for (FStringView url : message.m_audioUrls)
		{
			DownloadFile(url);
		}
	}
}

void UVoxtaAudioPlayback::DownloadFile(FStringView url)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("GET");
	FString fullUrl = FString::Format(*FString(TEXT("http://{0}:{1}{2}")), { m_hostAddress, m_hostPort, url });

	HttpRequest->SetURL(fullUrl);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UVoxtaAudioPlayback::OnDownloadComplete);
	HttpRequest->ProcessRequest();
}

void UVoxtaAudioPlayback::OnDownloadComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TArray<uint8> DownloadedData = response->GetContent();
		// Now you can save this data to a .wav file
		FFileHelper::SaveArrayToFile(DownloadedData, *FPaths::ProjectDir().Append(TEXT("/SavedFile.wav")));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("File download failed."));
	}
}