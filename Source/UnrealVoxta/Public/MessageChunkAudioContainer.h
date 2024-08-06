// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundWaveProcedural.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "RuntimeAudioImporter/RuntimeAudioImporterLibrary.h"
#include "OVRLipSyncFrame.h"

class MessageChunkAudioContainer
{
public:
	MessageChunkAudioContainer(const FString& fullUrl, TFunction<void(const MessageChunkAudioContainer* newState)> callback);

	void DownloadAsync();
	void Cleanup();

	USoundWaveProcedural* m_soundWave;
	UOVRLipSyncFrameSequence* m_frameSequence;

private:
	const FString m_downloadUrl;
	const TFunction<void(const MessageChunkAudioContainer* finishedChunk)> onFinished;

	void OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnImportComplete(TArray<uint8> buffer, UImportedSoundWave* soundWave);
	void GenerateOvrLipSync(const TArray<uint8>& rawSamples);
};
