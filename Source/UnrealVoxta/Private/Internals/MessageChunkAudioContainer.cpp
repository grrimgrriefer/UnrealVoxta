// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "MessageChunkAudioContainer.h"
#include "LipSyncGenerator.h"
#include "RuntimeAudioImporter/RuntimeAudioImporterLibrary.h"

MessageChunkAudioContainer::MessageChunkAudioContainer(const FString& fullUrl,
	LipSyncType lipSyncType,
	Audio2FaceRESTHandler& A2FRestHandler,
	TFunction<void(const MessageChunkAudioContainer* newState)> callback,
	int id) :
	m_index(id),
	m_lipSyncType(lipSyncType),
	m_downloadUrl(fullUrl),
	m_A2FRestHandler(A2FRestHandler),
	onStateChanged(callback)
{
}

void MessageChunkAudioContainer::CleanupData()
{
	m_soundWave->RemoveFromRoot();
	if (m_lipSyncType != LipSyncType::None)
	{
		m_lipSyncData->CleanupData();
	}
	m_rawData.Empty();
	m_state = MessageChunkState::CleanedUp;
	UE_LOG(LogTemp, Log, TEXT("Cleaned up MessageChunkAudioContainer."));
}

TArray<uint8> MessageChunkAudioContainer::GetRawData()
{
	return m_rawData;
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
			if (m_lipSyncType == LipSyncType::None)
			{
				UpdateState(MessageChunkState::ReadyForPlayback);
			}
			else
			{
				GenerateLipSync();
			}
			break;
		case MessageChunkState::Busy:
		case MessageChunkState::ReadyForPlayback:
		default:
			break;
	}
}

template<class T>
T* MessageChunkAudioContainer::GetLipSyncDataPtr() const
{
	return StaticCast<T*>(m_lipSyncData);
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
	if (m_state != MessageChunkState::Idle_Imported)
	{
		return;
	}
	UpdateState(MessageChunkState::Busy);

	switch (m_lipSyncType)
	{
		case LipSyncType::OVRLipSync:
#if WITH_OVRLIPSYNC
			LipSyncGenerator::GenerateOVRLipSyncData(m_rawData,
				[this] (ULipSyncDataOVR* lipsyncData)
				{
					m_lipSyncData = Cast<ILipSyncDataBase>(MoveTemp(lipsyncData));
					UpdateState(MessageChunkState::ReadyForPlayback);
				});
#endif
			break;
		case LipSyncType::Audio2Face:
			if (m_A2FRestHandler.IsBusy())
			{
				m_state = MessageChunkState::Idle_Imported; // TODO find a more clean way to do this
				return;
			}
			LipSyncGenerator::GenerateA2FLipSyncData(m_rawData,
				m_A2FRestHandler, [this] (ULipSyncDataA2F* lipsyncData)
				{
					m_lipSyncData = Cast<ILipSyncDataBase>(MoveTemp(lipsyncData));
					UpdateState(MessageChunkState::ReadyForPlayback);
				});
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("No built-in support yet for lipsync that isn't OVR or A2F."));
			break;
	}
}

void MessageChunkAudioContainer::OnRequestComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful)
{
	m_rawData = response->GetContent();
	UpdateState(MessageChunkState::Idle_Downloaded);
}

void MessageChunkAudioContainer::OnImportComplete(USoundWaveProcedural* soundWave)
{
	m_soundWave = soundWave;
	m_soundWave->AddToRoot();
	UpdateState(MessageChunkState::Idle_Imported);
}

void MessageChunkAudioContainer::UpdateState(MessageChunkState newState)
{
	if (m_state == MessageChunkState::CleanedUp)
	{
		UE_LOG(LogTemp, Error, TEXT("Some process was still running on the MessageChunkAudioContainer after it was cleaned up."));
		return;
	}

	if (newState == MessageChunkState::ReadyForPlayback)
	{
		m_rawData.Empty(); // save some memory
	}

	m_state = newState;
	if (m_state != MessageChunkState::Busy)
	{
		onStateChanged(this);
	}
}