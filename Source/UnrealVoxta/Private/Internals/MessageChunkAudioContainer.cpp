// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "MessageChunkAudioContainer.h"
#include "LipSyncGenerator.h"
#include "RuntimeAudioImporter/RuntimeAudioImporterLibrary.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Audio2FaceRESTHandler.h"
#include "Sound/SoundWaveProcedural.h"

MessageChunkAudioContainer::MessageChunkAudioContainer(const FString& fullUrl,
	LipSyncType lipSyncType,
	Audio2FaceRESTHandler* A2FRestHandler,
	TFunction<void(const MessageChunkAudioContainer* newState)> callback,
	int id) :
	INDEX(id),
	LIP_SYNC_TYPE(lipSyncType),
	FULL_DOWNLOAD_URL(fullUrl),
	ON_STATE_CHANGED(callback),
	m_A2FRestHandler(A2FRestHandler)
{
}

void MessageChunkAudioContainer::CleanupData()
{
	m_soundWave->RemoveFromRoot();
	if (LIP_SYNC_TYPE != LipSyncType::None)
	{
		m_lipSyncData->CleanupData();
	}
	m_rawAudioData.Empty();
	m_state = MessageChunkState::CleanedUp;
	UE_LOG(LogTemp, Log, TEXT("Cleaned up MessageChunkAudioContainer."));
}

const TArray<uint8>& MessageChunkAudioContainer::GetRawAudioData() const
{
	return m_rawAudioData;
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
			if (LIP_SYNC_TYPE == LipSyncType::None)
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
const T* MessageChunkAudioContainer::GetLipSyncData() const
{
	return StaticCast<const T*>(m_lipSyncData);
}

const MessageChunkState& MessageChunkAudioContainer::GetCurrentState() const
{
	return m_state;
}

USoundWaveProcedural* MessageChunkAudioContainer::GetSoundWave() const
{
	return m_soundWave;
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
	httpRequest->SetURL(FULL_DOWNLOAD_URL);
	httpRequest->OnProcessRequestComplete().BindLambda([Self = TWeakPtr<MessageChunkAudioContainer>(AsShared())]
	(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful)
		{
			if (TSharedPtr<MessageChunkAudioContainer> sharedSelf = Self.Pin())
			{
				sharedSelf->m_rawAudioData = response->GetContent();
				sharedSelf->UpdateState(MessageChunkState::Idle_Downloaded);
			}
		});
	httpRequest->ProcessRequest();
}

void MessageChunkAudioContainer::ImportData()
{
	URuntimeAudioImporterLibrary::ImportAudioFromBuffer(TArray64<uint8>(m_rawAudioData),
		[Self = TWeakPtr<MessageChunkAudioContainer>(AsShared())] (UImportedSoundWave* soundWave)
		{
			if (TSharedPtr<MessageChunkAudioContainer> sharedSelf = Self.Pin())
			{
				sharedSelf->m_soundWave = soundWave;
				sharedSelf->m_soundWave->AddToRoot();
				sharedSelf->UpdateState(MessageChunkState::Idle_Imported);
			}
		});
}

void MessageChunkAudioContainer::GenerateLipSync()
{
	if (m_state != MessageChunkState::Idle_Imported)
	{
		return;
	}
	UpdateState(MessageChunkState::Busy);

	switch (LIP_SYNC_TYPE)
	{
		case LipSyncType::OVRLipSync:
#if WITH_OVRLIPSYNC
			LipSyncGenerator::GenerateOVRLipSyncData(m_rawAudioData,
				[Self = TWeakPtr<MessageChunkAudioContainer>(AsShared())] (ULipSyncDataOVR* lipsyncData)
				{
					if (TSharedPtr<MessageChunkAudioContainer> sharedSelf = Self.Pin())
					{
						sharedSelf->m_lipSyncData = Cast<ILipSyncDataBase>(MoveTemp(lipsyncData));
						sharedSelf->UpdateState(MessageChunkState::ReadyForPlayback);
					}
				});
#endif
			break;
		case LipSyncType::Audio2Face:
			if (m_A2FRestHandler->IsBusy())
			{
				m_state = MessageChunkState::Idle_Imported; // TODO find a more clean way to do this
				return;
			}
			LipSyncGenerator::GenerateA2FLipSyncData(m_rawAudioData, m_A2FRestHandler,
				[Self = TWeakPtr<MessageChunkAudioContainer>(AsShared())] (ULipSyncDataA2F* lipsyncData)
				{
					if (TSharedPtr<MessageChunkAudioContainer> sharedSelf = Self.Pin())
					{
						sharedSelf->m_lipSyncData = Cast<ILipSyncDataBase>(MoveTemp(lipsyncData));
						sharedSelf->UpdateState(MessageChunkState::ReadyForPlayback);
					}
				});
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("No built-in support yet for lipsync that isn't OVR or A2F."));
			break;
	}
}

void MessageChunkAudioContainer::UpdateState(MessageChunkState newState)
{
	if (m_state == MessageChunkState::CleanedUp)
	{
		UE_LOG(LogTemp, Error, TEXT("Some process was still running on the MessageChunkAudioContainer after it was cleaned up."));
		return;
	}

	m_state = newState;
	if (m_state != MessageChunkState::Busy)
	{
		ON_STATE_CHANGED(this);
	}
}