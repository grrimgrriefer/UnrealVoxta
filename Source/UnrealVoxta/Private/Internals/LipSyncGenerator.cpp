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

void LipSyncGenerator::GenerateA2FLipSyncData(const TArray<uint8>& rawAudioData, TFunction<void(ULipSyncDataA2F*)> callback)
{
	UE_LOG(LogTemp, Error, TEXT("TODO"));

	ULipSyncDataA2F* data = NewObject<ULipSyncDataA2F>();
	TArray<TimedWeightSample> samples;
	data->SetA2FCurveWeights(samples);
	data->AddToRoot();
	callback(data);

	/*

	Required HTTP calls, verified with Swagger API

----------------------------------------------------------------------
1. get status

curl -X 'GET' \
  'http://localhost:8011/status' \
  -H 'accept: application/json'

"OK"

----------------------------------------------------------------------
2. load usd into stage

curl -X 'POST' \
  'http://localhost:8011/A2F/USD/Load' \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "file_name": "D:\\Documents\\Unreal Projects\\VoxtaTestProject\\Plugins\\UnrealVoxta\\Content\\test.usd"
}'

{
  "status": "OK",
  "message": "Succeeded"
}

----------------------------------------------------------------------
3. get name of instance so we can reference it later

curl -X 'GET' \
  'http://localhost:8011/A2F/GetInstances' \
  -H 'accept: application/json'

{
  "status": "OK",
  "result": {
	"fullface_instances": [
	  "/World/audio2face/CoreFullface"
	]
  },
  "message": "Succeeded"
}

----------------------------------------------------------------------
4. get possible settings

curl -X 'POST' \
  'http://localhost:8011/A2F/GetSettingNames' \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "a2f_instance": "/World/audio2face/CoreFullface"
}'

----------------------------------------------------------------------
5. confirm livelink is disabled

curl -X 'POST' \
  'http://localhost:8011/A2F/GetSettings' \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "a2f_instance": "/World/audio2face/CoreFullface",
  "settings": [
	"a2e_streaming_live_mode"
  ]
}'

{
  "status": "OK",
  "result": {
	"a2e_streaming_live_mode": false
}

----------------------------------------------------------------------
6. get player

curl -X 'GET' \
  'http://localhost:8011/A2F/Player/GetInstances' \
  -H 'accept: application/json'

{
  "status": "OK",
  "result": {
	"regular": [
	  "/World/audio2face/Player"
	],
	"streaming": []
  },
  "message": "Suceeded to retrieve Player instances"
}

----------------------------------------------------------------------
7. set root path

curl -X 'POST' \
  'http://localhost:8011/A2F/Player/SetRootPath' \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "a2f_player": "/World/audio2face/Player",
  "dir_path": "D:\\Documents\\Unreal Projects\\VoxtaTestProject\\Plugins\\UnrealVoxta\\Content"
}'

{
  "status": "OK",
  "message": "Set the audio file root to D:\\Documents\\Unreal Projects\\VoxtaTestProject\\Plugins\\UnrealVoxta\\Content"
}

----------------------------------------------------------------------
8. get playable tracks

curl -X 'POST' \
  'http://localhost:8011/A2F/Player/GetTracks' \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "a2f_player": "/World/audio2face/Player"
}'

{
  "status": "OK",
  "result": [
	"speak.wav"
  ],
  "message": "Suceeded"
}

----------------------------------------------------------------------
9. set tracks

curl -X 'POST' \
  'http://localhost:8011/A2F/Player/SetTrack' \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "a2f_player": "/World/audio2face/Player",
  "file_name": "speak.wav",
  "time_range": [
	0,
	-1
  ]
}'

{
  "status": "OK",
  "message": "Set track to speak.wav"
}

----------------------------------------------------------------------
10. get blendshape solver

curl -X 'GET' \
  'http://localhost:8011/A2F/Exporter/GetBlendShapeSolvers' \
  -H 'accept: application/json'

{
  "status": "OK",
  "result": [
	"/World/audio2face/BlendshapeSolve"
  ]
}

----------------------------------------------------------------------
11. generate blendshapes

curl -X 'POST' \
  'http://localhost:8011/A2F/Exporter/ExportBlendshapes' \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "solver_node": "/World/audio2face/BlendshapeSolve",
  "export_directory": "D:\\Documents\\Unreal Projects\\VoxtaTestProject\\Plugins\\UnrealVoxta\\Content",
  "file_name": "blendshapes1",
  "format": "json",
  "batch": false,
  "fps": 30
}'

{
  "status": "OK"
}

	*/
}