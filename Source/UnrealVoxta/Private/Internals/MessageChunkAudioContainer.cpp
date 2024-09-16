// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "MessageChunkAudioContainer.h"
#include "Logging/StructuredLog.h"
#include "VoxtaDefines.h"
#include "LipSyncGenerator.h"
#include "RuntimeAudioImporter/RuntimeAudioImporterLibrary.h"
#include "HttpModule.h"
#include "Interfaces/IHttpBase.h"
#include "Interfaces/IHttpRequest.h"
#include "Audio2FaceRESTHandler.h"
#include "Sound/SoundWaveProcedural.h"
#include "LipSyncDataBase.h"

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

void MessageChunkAudioContainer::Continue()
{
	switch (m_state)
	{
		case MessageChunkState::Idle:
			DownloadData();
			break;
		case MessageChunkState::Idle_Downloaded:
			ProcessAudioData();
			break;
		case MessageChunkState::Idle_Processed:
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
			UE_LOGFMT(VoxtaLog, Warning, "Cannot continue the MessageChunkAudioContainer, please check the current "
				"state before attempting to continue into the next state. Current state: {0}",
				UEnum::GetValueAsString(m_state));
			break;
	}
}

void MessageChunkAudioContainer::CleanupData()
{
	UE_LOGFMT(VoxtaLog, Log, "Cleaning up MessageChunkAudioContainer for index: {0}", INDEX);

	m_soundWave->RemoveFromRoot();
	if (LIP_SYNC_TYPE != LipSyncType::None)
	{
		m_lipSyncData->CleanupData();
	}
	m_rawAudioData.Empty();
	m_state = MessageChunkState::CleanedUp;
}

const TArray<uint8>& MessageChunkAudioContainer::GetRawAudioData() const
{
	return m_rawAudioData;
}

template<class T>
const T* MessageChunkAudioContainer::GetLipSyncData() const
{
	return StaticCast<const T*>(m_lipSyncData);
}

MessageChunkState MessageChunkAudioContainer::GetCurrentState() const
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
		UE_LOGFMT(VoxtaLog, Warning, "Cannot download data for MessageChunkAudioContainer as it was not Idle. "
			"Current state: {0}", UEnum::GetValueAsString(m_state));
		return;
	}
	UpdateState(MessageChunkState::Busy);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> httpRequest = FHttpModule::Get().CreateRequest();
	httpRequest->SetVerb("GET");
	httpRequest->SetURL(FULL_DOWNLOAD_URL);
	httpRequest->OnProcessRequestComplete().BindLambda([Self = TWeakPtr<MessageChunkAudioContainer>(AsShared())]
	(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful)
		{
			if (bWasSuccessful)
			{
				if (TSharedPtr<MessageChunkAudioContainer> sharedSelf = Self.Pin())
				{
					sharedSelf->m_rawAudioData = response->GetContent();
					UE_LOGFMT(VoxtaLog, Log, "Sucessfully downloaded audio data from: {0}", request->GetURL());
					sharedSelf->UpdateState(MessageChunkState::Idle_Downloaded);
				}
				else
				{
					UE_LOGFMT(VoxtaLog, Error, "Downloaded audio data from: {0} "
						"But the messageChunkContainer was destroyed?", request->GetURL());
				}
			}
			else
			{
				UE_LOGFMT(VoxtaLog, Error, "Failed to download audio data from: {0}", request->GetURL());
			}
		});

	UE_LOGFMT(VoxtaLog, Log, "Attempting to request audio data for index {0}, from url: {1}", INDEX, FULL_DOWNLOAD_URL);
	httpRequest->ProcessRequest();
}

void MessageChunkAudioContainer::ProcessAudioData()
{
	if (m_state != MessageChunkState::Idle_Downloaded)
	{
		UE_LOGFMT(VoxtaLog, Warning, "Cannot process the raw audiodata for MessageChunkAudioContainer as it was not "
			"in Idle_Downloaded. Current state: {0}", UEnum::GetValueAsString(m_state));
		return;
	}

	UE_LOGFMT(VoxtaLog, Log, "Attempting to process raw audio data into UImportedSoundWave for index {0}.", INDEX);
	URuntimeAudioImporterLibrary::ImportAudioFromBuffer(TArray64<uint8>(m_rawAudioData),
		[Self = TWeakPtr<MessageChunkAudioContainer>(AsShared())] (UImportedSoundWave* soundWave)
		{
			if (soundWave)
			{
				if (TSharedPtr<MessageChunkAudioContainer> sharedSelf = Self.Pin())
				{
					sharedSelf->m_soundWave = soundWave;
					sharedSelf->m_soundWave->AddToRoot();

					UE_LOGFMT(VoxtaLog, Error, "Sucessfully processed raw audio data into UImportedSoundWave for "
						"index {0}", sharedSelf->INDEX);
					sharedSelf->UpdateState(MessageChunkState::Idle_Processed);
				}
			}
			else
			{
				UE_LOGFMT(VoxtaLog, Error, "Failed to process raw audio data into UImportedSoundWave.");
			}
		});
}

void MessageChunkAudioContainer::GenerateLipSync()
{
	if (m_state != MessageChunkState::Idle_Processed)
	{
		UE_LOGFMT(VoxtaLog, Warning, "Cannot generate lipsync data for MessageChunkAudioContainer as it was not "
			"in Idle_Processed. Current state: {0}", UEnum::GetValueAsString(m_state));
		return;
	}

	UpdateState(MessageChunkState::Busy);

	switch (LIP_SYNC_TYPE)
	{
		case LipSyncType::OVRLipSync:
#if WITH_OVRLIPSYNC
			UE_LOGFMT(VoxtaLog, Log, "Starting OVR lipsync generation for MessageChunkAudioContainer with index: {0}",
				INDEX);
			LipSyncGenerator::GenerateOVRLipSyncData(m_rawAudioData,
				[Self = TWeakPtr<MessageChunkAudioContainer>(AsShared())] (ULipSyncDataOVR* lipsyncData)
				{
					if (lipsyncData)
					{
						if (TSharedPtr<MessageChunkAudioContainer> sharedSelf = Self.Pin())
						{
							sharedSelf->m_lipSyncData = Cast<ILipSyncDataBase>(MoveTemp(lipsyncData));

							UE_LOGFMT(VoxtaLog, Error, "Sucessfully genrated OVR lipsyncdata for "
								"index {0}", sharedSelf->INDEX);
							sharedSelf->UpdateState(MessageChunkState::ReadyForPlayback);
						}
						else
						{
							UE_LOGFMT(VoxtaLog, Error, "Generated OVR lipsyncdata, but the messageChunkContainer "
								"was destroyed?");
						}
					}
					else
					{
						UE_LOGFMT(VoxtaLog, Error, "Failed to generate OVR lipsyncdata for MessageChunkAudioContainer.");
					}
				});
#endif
			break;
		case LipSyncType::Audio2Face:
			if (m_A2FRestHandler->IsBusy())
			{
				UE_LOGFMT(VoxtaLog, Warning, "A2F is busy at the moment, aborting lipsync generattion request, moving "
					"the state back into Idle_Processed so it can be attempted later.",
					INDEX);
				m_state = MessageChunkState::Idle_Processed; // TODO find a more clean way to do this
				return;
			}

			UE_LOGFMT(VoxtaLog, Log, "Starting A2F lipsync generation for MessageChunkAudioContainer with index: {0}",
				INDEX);
			LipSyncGenerator::GenerateA2FLipSyncData(m_rawAudioData, m_A2FRestHandler,
				[Self = TWeakPtr<MessageChunkAudioContainer>(AsShared())] (ULipSyncDataA2F* lipsyncData)
				{
					if (lipsyncData)
					{
						if (TSharedPtr<MessageChunkAudioContainer> sharedSelf = Self.Pin())
						{
							sharedSelf->m_lipSyncData = Cast<ILipSyncDataBase>(MoveTemp(lipsyncData));

							UE_LOGFMT(VoxtaLog, Error, "Sucessfully genrated A2F lipsyncdata for "
								"index {0}", sharedSelf->INDEX);
							sharedSelf->UpdateState(MessageChunkState::ReadyForPlayback);
						}
						else
						{
							UE_LOGFMT(VoxtaLog, Error, "Generated A2F lipsyncdata, but the messageChunkContainer "
								"was destroyed?");
						}
					}
					else
					{
						UE_LOGFMT(VoxtaLog, Error, "Failed to generate A2F lipsyncdata for MessageChunkAudioContainer.");
					}
				});
			break;
		default:
			UE_LOGFMT(VoxtaLog, Error, "No built-in support yet for lipsync that isn't OVR or A2F.");
			break;
	}
}

void MessageChunkAudioContainer::UpdateState(MessageChunkState newState)
{
	if (m_state == MessageChunkState::CleanedUp)
	{
		UE_LOGFMT(VoxtaLog, Error, "Some process was still running on the MessageChunkAudioContainer after "
			"it was cleaned up.");
		return;
	}

	m_state = newState;
	if (m_state != MessageChunkState::Busy)
	{
		ON_STATE_CHANGED(this);
	}
}