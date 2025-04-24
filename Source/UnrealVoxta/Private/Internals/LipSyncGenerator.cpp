// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "LipSyncGenerator.h"
#include "Logging/StructuredLog.h"
#include "VoxtaDefines.h"
#include "Serialization/JsonReader.h"
#if WITH_OVRLIPSYNC
#include "OVRLipSyncContextWrapper.h"
#include "OVRLipSyncFrame.h"
#include "LipSyncDataOVR.h"
#endif
#include "Audio2FaceRESTHandler.h"
#include "LipSyncDataA2F.h"
#include "LipSyncDataCustom.h"
#include "Interfaces/IPluginManager.h"

#if WITH_OVRLIPSYNC
void LipSyncGenerator::GenerateOVRLipSyncData(const TArray<uint8>& rawAudioData,
	TFunction<void(ULipSyncDataOVR*)> callback)
{
	if (rawAudioData.Num() <= 44)
	{
		UE_LOGFMT(VoxtaLog, Error, "Invalid rawAudioData, cannot generate OVR lipsync data.");
		callback(nullptr);
		return;
	}

	FWaveModInfo waveInfo;
	uint8* waveData = const_cast<uint8*>(rawAudioData.GetData());

	if (waveInfo.ReadWaveInfo(waveData, rawAudioData.Num()))
	{
		int16_t* pcmData = reinterpret_cast<int16_t*>(waveData + 44);
		FString modelPath = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("OVRLipSync"),
			TEXT("OfflineModel"), TEXT("ovrlipsync_offline_model.pb"));

		Async(EAsyncExecution::Thread, [WaveInfo = waveInfo, ModelPath = modelPath,
			PCMData = pcmData, Callback1 = callback] ()
		{
			int32 numChannels = *WaveInfo.pChannels;
			int32 sampleRate = *WaveInfo.pSamplesPerSec;
			auto pcmDataSize = WaveInfo.SampleDataSize / sizeof(int16_t);
			int32 chunkSizeSamples = static_cast<int32>(sampleRate * (1.f / 100.f));
			int32 chunkSize = numChannels * chunkSizeSamples;
			int bufferSize = 4096;

			UOVRLipSyncContextWrapper context(ovrLipSyncContextProvider_Enhanced, sampleRate, bufferSize, ModelPath);
			float laughterScore = 0.0f;
			int32_t FrameDelayInMs = 0;
			TArray<float> viseme;
			TArray<TTuple<TArray<float>, float>> frames;
			for (uint64 offs = 0; (offs + chunkSize) < pcmDataSize; offs += chunkSize)
			{
				viseme.Reset();
				context.ProcessFrame(PCMData + offs, chunkSizeSamples, viseme, laughterScore, FrameDelayInMs,
					numChannels > 1);
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
				UE_LOGFMT(VoxtaLog, Log, "Sucessfully generated OVR lipsync data: {0} frames of data.", Frames.Num());

				Callback2(data);
			});
		});
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Invalid wave header detected, cannot generate OVR lipsync data.");
		callback(nullptr);
	}
}
#endif

void LipSyncGenerator::GenerateA2FLipSyncData(const TArray<uint8>& rawAudioData, Audio2FaceRESTHandler* A2FRestHandler,
	TFunction<void(ULipSyncDataA2F*)> callback)
{
	FString guid = FGuid::NewGuid().ToString();
	FString cacheFolder = FString::Format(*FString(TEXT("{0}\\A2FCache")),
		{ IPluginManager::Get().FindPlugin("UnrealVoxta")->GetContentDir() });
	FString wavName = FString::Format(*FString(TEXT("A2FCachedData{0}.wav")), { guid });
	FString jsonName = FString::Format(*FString(TEXT("A2FCachedData{0}")), { guid });
	FString jsonImportName = FString::Format(*FString(TEXT("{0}_bsweight.json")), { jsonName });

	FWaveModInfo waveInfo;
	uint8* waveData = const_cast<uint8*>(rawAudioData.GetData());

	if (waveInfo.ReadWaveInfo(waveData, rawAudioData.Num()))
	{
		if (!IFileManager::Get().DirectoryExists(*cacheFolder))
		{
			if (!IFileManager::Get().MakeDirectory(*cacheFolder, true))
			{
				UE_LOGFMT(VoxtaLog, Error, "Failed to create wav data folder for A2F processing.");
				callback(nullptr);
				return;
			}
		}
		IFileHandle* FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(
			*FPaths::Combine(cacheFolder, wavName));

		if (FileHandle)
		{
			FileHandle->Write(waveData, waveInfo.SampleDataStart - waveData);
			FileHandle->Write(waveInfo.SampleDataStart, waveInfo.SampleDataSize);
			delete FileHandle;
		}
		else
		{
			UE_LOGFMT(VoxtaLog, Error, "Failed to write wav data to disk for A2F processing.");
			callback(nullptr);
			return;
		}
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Invalid wave header detected, cannot generate OVR lipsync data.");
		callback(nullptr);
		return;
	}

	A2FRestHandler->GetBlendshapes(wavName, cacheFolder, jsonName,
		[Callback = callback, JsonFullPath = FPaths::Combine(cacheFolder, jsonImportName)]
		(FString shapesFile, bool success)
		{
			if (!success)
			{
				UE_LOGFMT(VoxtaLog, Error, "A2F failed to generate blendshape curves from the audiofile, aborting.");
				Callback(nullptr);
				return;
			}

			FString FileContents;
			FFileHelper::LoadFileToString(FileContents, *JsonFullPath);

			TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
			const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileContents);

			if (!(JsonObject.IsValid() && FJsonSerializer::Deserialize(JsonReader, JsonObject)))
			{
				UE_LOGFMT(VoxtaLog, Error, "Failed to parse JSON A2F LipSyncData, this should never happen?");
				Callback(nullptr);
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
			UE_LOGFMT(VoxtaLog, Log, "Sucessfully generated A2F lipsync data: {0} frames of data.", numFrames);

			Callback(data);
		});
}

ULipSyncDataCustom* LipSyncGenerator::GenerateCustomLipSyncData()
{
	ULipSyncDataCustom* data = NewObject<ULipSyncDataCustom>();
	data->AddToRoot();
	return data;
}