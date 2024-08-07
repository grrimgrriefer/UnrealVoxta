// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "MessageChunkAudioContainer.h"
#include "OVRLipSyncContextWrapper.h"
#include "RuntimeAudioImporter/RuntimeAudioImporterLibrary.h"

MessageChunkAudioContainer::MessageChunkAudioContainer(const FString& fullUrl,
	TFunction<void(const MessageChunkAudioContainer* newState)> callback,
	int id)
	: m_downloadUrl(fullUrl),
	m_id(id),
	onStateChanged(callback)
{
}

void MessageChunkAudioContainer::CleanupData()
{
	m_soundWave->RemoveFromRoot();
	m_frameSequence->RemoveFromRoot();
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
			GenerateOvrLipSync();
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

void MessageChunkAudioContainer::GenerateOvrLipSync()
{
	if (m_rawData.Num() <= 44)
	{
		return;
	}

	FWaveModInfo waveInfo;
	uint8* waveData = const_cast<uint8*>(m_rawData.GetData());

	if (waveInfo.ReadWaveInfo(waveData, m_rawData.Num()))
	{
		int32 numChannels = *waveInfo.pChannels;
		int32 sampleRate = *waveInfo.pSamplesPerSec;
		auto pcmDataSize = waveInfo.SampleDataSize / sizeof(int16_t);
		int16_t* pcmData = reinterpret_cast<int16_t*>(waveData + 44);
		int32 chunkSizeSamples = static_cast<int32>(sampleRate * (1.f / 100.f));
		int32 chunkSize = numChannels * chunkSizeSamples;
		int bufferSize = 4096;

		FString modelPath = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("OVRLipSync"),
			TEXT("OfflineModel"), TEXT("ovrlipsync_offline_model.pb"));

		Async(EAsyncExecution::Thread, [&, SampleRate = sampleRate, BufferSize = bufferSize, ModelPath = modelPath,
			ChunkSize = chunkSize, PCMDataSize = pcmDataSize, PCMData = pcmData, ChunkSizeSamples = chunkSizeSamples,
			NumChannels = numChannels] ()
		{
			UOVRLipSyncFrameSequence* sequence = NewObject<UOVRLipSyncFrameSequence>();
			sequence->AddToRoot();
			UOVRLipSyncContextWrapper context(ovrLipSyncContextProvider_Enhanced, SampleRate, BufferSize, ModelPath);
			float LaughterScore = 0.0f;
			int32_t FrameDelayInMs = 0;
			TArray<float> Visemes;
			for (int offs = 0; offs + ChunkSize < PCMDataSize; offs += ChunkSize)
			{
				context.ProcessFrame(PCMData + offs, ChunkSizeSamples, Visemes, LaughterScore, FrameDelayInMs, NumChannels > 1);
				sequence->Add(Visemes, LaughterScore);
			}
			AsyncTask(ENamedThreads::GameThread, [&, Sequence = sequence] ()
			{
				m_frameSequence = Sequence;
				UpdateState(MessageChunkState::ReadyForPlayback);
			});
		});
	}
}

void MessageChunkAudioContainer::OnRequestComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful)
{
	m_rawData = response->GetContent();
	UpdateState(MessageChunkState::Idle_Downloaded);
}

void MessageChunkAudioContainer::OnImportComplete(UImportedSoundWave* soundWave)
{
	m_soundWave = soundWave;
	m_soundWave->AddToRoot();
	UpdateState(MessageChunkState::Idle_Imported);
}

void MessageChunkAudioContainer::UpdateState(MessageChunkState newState)
{
	m_state = newState;
	if (m_state != MessageChunkState::Busy)
	{
		onStateChanged(this);
	}
}