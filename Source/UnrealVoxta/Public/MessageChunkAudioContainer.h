// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundWaveProcedural.h"
#include "OVRLipSyncFrame.h"
#include "Runtime/Online/HTTP/Public/Http.h"

enum class MessageChunkState : uint8
{
	Idle,
	Idle_Downloaded,
	Idle_Imported,
	Busy,
	ReadyForPlayback
};

class MessageChunkAudioContainer
{
public:
	MessageChunkAudioContainer(const FString& fullUrl, TFunction<void(const MessageChunkAudioContainer* newState)> callback, int id);

	void Continue();
	void CleanupData();

	const int m_id;
	USoundWaveProcedural* m_soundWave;
	UOVRLipSyncFrameSequence* m_frameSequence;
	MessageChunkState m_state = MessageChunkState::Idle;

private:
	const FString m_downloadUrl;
	const TFunction<void(const MessageChunkAudioContainer* chunk)> onStateChanged;
	TArray<uint8> m_rawData;

	void DownloadData();
	void ImportData();
	void GenerateOvrLipSync();
	void OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnImportComplete(UImportedSoundWave* soundWave);
	void UpdateState(MessageChunkState newState);
};
