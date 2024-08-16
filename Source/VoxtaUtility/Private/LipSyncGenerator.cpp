// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "LipSyncGenerator.h"
#if WITH_OVRLIPSYNC
#include "OVRLipSyncContextWrapper.h"
#include "OVRLipSyncFrame.h"
#endif

void LipSyncGenerator::GenerateLipSync(const LipSyncType lipSyncType, const TArray<uint8>& rawAudioData, TFunction<void(FLipSyncData)> callback)
{
	switch (lipSyncType)
	{
		case LipSyncType::Custom:
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Callback = callback] (float DeltaTime)
				{
					Callback(FLipSyncData()); // Leave one tick before triggering callback to keep blueprints simple
					return false;
				}));
			break;
		case LipSyncType::OVRLipSync:
#if WITH_OVRLIPSYNC
			GenerateOVRLipSyncData(rawAudioData, callback);
#else
			checkNoEntry();
#endif
			break;
		default:
			checkNoEntry();
			break;
	}
}

#if WITH_OVRLIPSYNC
void LipSyncGenerator::GenerateOVRLipSyncData(const TArray<uint8>& rawAudioData, TFunction<void(FLipSyncData)> callback)
{
	if (rawAudioData.Num() <= 44)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid rawAudioData, cannot generate OVR lipsync."));
		checkNoEntry();
	}

	FWaveModInfo waveInfo;
	uint8* waveData = const_cast<uint8*>(rawAudioData.GetData());

	if (waveInfo.ReadWaveInfo(waveData, rawAudioData.Num()))
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
			NumChannels = numChannels, Callback = callback] ()
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
				FLipSyncData data(sequence);
				Callback(data);
			});
		});
	}
}
#endif