// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "MessageChunkAudioContainer.h"
#include "LipSyncGenerator.h"
#include "RuntimeAudioImporter/RuntimeAudioImporterLibrary.h"

MessageChunkAudioContainer::MessageChunkAudioContainer(const FString& fullUrl,
	LipSyncType lipSyncType,
	TFunction<void(const MessageChunkAudioContainer* newState)> callback,
	int id) :
	m_id(id),
	m_lipSyncType(lipSyncType),
	m_downloadUrl(fullUrl),
	onStateChanged(callback)
{
}

void MessageChunkAudioContainer::CleanupData()
{
	m_soundWave->RemoveFromRoot();
	m_lipSyncData.CleanupData();
}

void MessageChunkAudioContainer::Continue()
{
	switch (m_state)
	{
		case MessageChunkState::Idle:
			DownloadData();
			break;
		case MessageChunkState::Idle_Downloaded:
			ImportData();
			break;
		case MessageChunkState::Idle_Imported:
			GenerateLipSync();
			break;
		case MessageChunkState::Busy:
		case MessageChunkState::ReadyForPlayback:
		default:
			break;
	}
}

void MessageChunkAudioContainer::DownloadData()
{
	if (m_state != MessageChunkState::Idle)
	{
		return;
	}
	UpdateState(MessageChunkState::Busy);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> httpRequest = FHttpModule::Get().CreateRequest();
	httpRequest->SetVerb("GET");
	httpRequest->SetURL(m_downloadUrl);
	httpRequest->OnProcessRequestComplete().BindRaw(this, &MessageChunkAudioContainer::OnRequestComplete);
	httpRequest->ProcessRequest();
}

void MessageChunkAudioContainer::ImportData()
{
	URuntimeAudioImporterLibrary::ImportAudioFromBuffer(TArray64<uint8>(m_rawData),
		[this] (UImportedSoundWave* soundWave) { OnImportComplete(soundWave); });
}

void MessageChunkAudioContainer::GenerateLipSync()
{
	LipSyncGenerator::GenerateLipSync(m_rawData,
		[this] (FLipSyncData lipsyncData) { OnLipSyncGenComplete(lipsyncData); });
}

void MessageChunkAudioContainer::OnRequestComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful)
{
	m_rawData = response->GetContent();
	UpdateState(MessageChunkState::Idle_Downloaded);
}

void MessageChunkAudioContainer::OnImportComplete(USoundWaveProcedural* soundWave)
{
	m_rawData.Empty();
	m_soundWave = soundWave;
	m_soundWave->AddToRoot();
	UpdateState(MessageChunkState::Idle_Imported);
}

void MessageChunkAudioContainer::OnLipSyncGenComplete(FLipSyncData lipSyncData)
{
	m_lipSyncData = lipSyncData;
	UpdateState(MessageChunkState::ReadyForPlayback);
}

void MessageChunkAudioContainer::UpdateState(MessageChunkState newState)
{
	m_state = newState;
	if (m_state != MessageChunkState::Busy)
	{
		onStateChanged(this);
	}
}