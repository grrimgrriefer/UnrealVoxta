// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "MessageChunkAudioContainer.h"
#include "OVRLipSyncContextWrapper.h"

MessageChunkAudioContainer::MessageChunkAudioContainer(const FString& fullUrl,
	TFunction<void(const MessageChunkAudioContainer* finishedChunk)> callback)
	: m_downloadUrl(fullUrl),
	onFinished(callback)
{
}

void MessageChunkAudioContainer::DownloadAsync()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> httpRequest = FHttpModule::Get().CreateRequest();
	httpRequest->SetVerb("GET");
	httpRequest->SetURL(m_downloadUrl);
	httpRequest->OnProcessRequestComplete().BindRaw(this, &MessageChunkAudioContainer::OnRequestComplete);
	httpRequest->ProcessRequest();
}

void MessageChunkAudioContainer::Cleanup()
{
	m_soundWave->RemoveFromRoot();
	m_frameSequence->RemoveFromRoot();
}

void MessageChunkAudioContainer::OnRequestComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful)
{
	URuntimeAudioImporterLibrary::ImportAudioFromBuffer(TArray64<uint8>(response->GetContent()),
		[this, Response = response] (UImportedSoundWave* soundWave) { OnImportComplete(Response->GetContent(), soundWave); });
}

void MessageChunkAudioContainer::OnImportComplete(TArray<uint8> buffer, UImportedSoundWave* soundWave)
{
	m_soundWave = soundWave;
	m_soundWave->AddToRoot();

	GenerateOvrLipSync(buffer);
}

void MessageChunkAudioContainer::GenerateOvrLipSync(const TArray<uint8>& rawSamples)
{
	if (rawSamples.Num() <= 44)
	{
		return;
	}

	FWaveModInfo waveInfo;
	uint8* waveData = const_cast<uint8*>(rawSamples.GetData());

	if (waveInfo.ReadWaveInfo(waveData, rawSamples.Num()))
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
			UOVRLipSyncFrameSequence* Sequence = NewObject<UOVRLipSyncFrameSequence>();
			UOVRLipSyncContextWrapper context(ovrLipSyncContextProvider_Enhanced, SampleRate, BufferSize, ModelPath);
			float LaughterScore = 0.0f;
			int32_t FrameDelayInMs = 0;
			TArray<float> Visemes;
			for (int offs = 0; offs + ChunkSize < PCMDataSize; offs += ChunkSize)
			{
				context.ProcessFrame(PCMData + offs, ChunkSizeSamples, Visemes, LaughterScore, FrameDelayInMs, NumChannels > 1);
				Sequence->Add(Visemes, LaughterScore);
			}
			AsyncTask(ENamedThreads::GameThread, [&] ()
			{
				m_frameSequence = Sequence;
				m_frameSequence->AddToRoot();
				onFinished(this);
			});
		});
	}
}