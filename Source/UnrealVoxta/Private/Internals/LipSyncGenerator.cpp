// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "LipSyncGenerator.h"
#if WITH_OVRLIPSYNC
#include "OVRLipSyncContextWrapper.h"
#include "OVRLipSyncFrame.h"
#include "LipSyncDataOVR.h"
#endif
#include "Serialization/JsonReader.h"
#include "Audio2FaceRESTHandler.h"
#include "LipSyncDataA2F.h"

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

void LipSyncGenerator::GenerateA2FLipSyncData(const TArray<uint8>& rawAudioData, Audio2FaceRESTHandler* A2FRestHandler, TFunction<void(ULipSyncDataA2F*)> callback)
{
	FString guid = FGuid::NewGuid().ToString();
	FString cacheFolder = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("UnrealVoxta"), TEXT("Content"), TEXT("A2FCache"));
	FString wavName = FString::Format(*FString(TEXT("A2FCachedData{0}.wav")), { guid });
	FString jsonName = FString::Format(*FString(TEXT("A2FCachedData{0}")), { guid });
	FString jsonImportName = FString::Format(*FString(TEXT("{0}_bsweight.json")), { jsonName });

	FWaveModInfo waveInfo;
	uint8* waveData = const_cast<uint8*>(rawAudioData.GetData());

	if (waveInfo.ReadWaveInfo(waveData, rawAudioData.Num()))
	{
		// Create a file handle
		IFileHandle* FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FPaths::Combine(cacheFolder, wavName));

		if (FileHandle)
		{
			// Write the WAV header
			FileHandle->Write(waveData, waveInfo.SampleDataStart - waveData);

			// Write the audio data
			FileHandle->Write(waveInfo.SampleDataStart, waveInfo.SampleDataSize);

			// Close the file
			delete FileHandle;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to open file for writing"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read wave info"));
	}

	A2FRestHandler->GetBlendshapes(wavName, cacheFolder, jsonName,
		[Callback = callback, JsonFullPath = FPaths::Combine(cacheFolder, jsonImportName)] (FString shapesFile, bool success)
		{
			FString FileContents;
			FFileHelper::LoadFileToString(FileContents, *JsonFullPath);

			TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
			const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileContents);

			if (!(JsonObject.IsValid() && FJsonSerializer::Deserialize(JsonReader, JsonObject)))
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON A2F LipSyncData"));
				return;
			}

			int32 fps;
			JsonObject->TryGetNumberField(TEXT("exportFps"), fps);
			int32 numFrames;
			JsonObject->TryGetNumberField(TEXT("numFrames"), numFrames);

			const TArray<TSharedPtr<FJsonValue>>* WeightMat;
			JsonObject->TryGetArrayField(TEXT("weightMat"), WeightMat);

			TArray<TArray<float>> curveValues;
			for (const TSharedPtr<FJsonValue>& Row : *WeightMat)
			{
				TArray<float> FloatRow;
				const TArray<TSharedPtr<FJsonValue>>& values = Row->AsArray();
				for (const TSharedPtr<FJsonValue>& Value : values)
				{
					FloatRow.Add(Value->AsNumber());
				}
				curveValues.Add(FloatRow);
			}

			ULipSyncDataA2F* data = NewObject<ULipSyncDataA2F>();
			data->SetA2FCurveWeights(curveValues, fps);
			data->AddToRoot();
			Callback(data);
		});
}