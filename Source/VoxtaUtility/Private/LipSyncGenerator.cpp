// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "LipSyncGenerator.h"
#if WITH_OVRLIPSYNC
#include "OVRLipSyncContextWrapper.h"
#include "OVRLipSyncFrame.h"
#endif

#if WITH_OVRLIPSYNC
void LipSyncGenerator::GenerateOVRLipSyncData(const TArray<uint8>& rawAudioData, TFunction<void(ULipSyncDataOVR*)> callback)
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

		Async(EAsyncExecution::Thread, [SampleRate = sampleRate, BufferSize = bufferSize, ModelPath = modelPath,
			ChunkSize = chunkSize, PCMDataSize = pcmDataSize, PCMData = pcmData, ChunkSizeSamples = chunkSizeSamples,
			NumChannels = numChannels, Callback1 = callback] ()
		{
			UOVRLipSyncContextWrapper context(ovrLipSyncContextProvider_Enhanced, SampleRate, BufferSize, ModelPath);
			float laughterScore = 0.0f;
			int32_t FrameDelayInMs = 0;
			TArray<float> viseme;
			TArray<TTuple<TArray<float>, float>> frames;
			for (int offs = 0; offs + ChunkSize < PCMDataSize; offs += ChunkSize)
			{
				context.ProcessFrame(PCMData + offs, ChunkSizeSamples, viseme, laughterScore, FrameDelayInMs, NumChannels > 1);
				frames.Emplace(viseme, laughterScore);
			}
			AsyncTask(ENamedThreads::GameThread, [Frames = frames, Callback2 = Callback1] ()
			{
				ULipSyncDataOVR* data = NewObject<ULipSyncDataOVR>();
				UOVRLipSyncFrameSequence* sequence = NewObject<UOVRLipSyncFrameSequence>(data);
				for (int i = 0; i < Frames.Num(); i++)
				{
					sequence->Add(Frames[i].Key, Frames[i].Value);
				}
				data->SetFrameSequence(sequence);
				data->AddToRoot();
				Callback2(data);
			});
		});
	}
}
#endif