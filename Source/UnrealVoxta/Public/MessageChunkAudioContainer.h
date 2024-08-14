// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundWaveProcedural.h"
#include "LipSyncData.h"
#include "Runtime/Online/HTTP/Public/Http.h"

enum class MessageChunkState : uint8
{
	Idle,
	Idle_Downloaded,
	Idle_Imported,
	Busy,
	ReadyForPlayback,
	CleanedUp
};

class MessageChunkAudioContainer
{
public:
	MessageChunkAudioContainer(const FString& fullUrl,
		LipSyncType lipSyncType,
		TFunction<void(const MessageChunkAudioContainer* newState)> callback,
		int id);

	void Continue();
	void CleanupData();
	TArray<uint8> GetRawData();

	const int m_id;
	const LipSyncType m_lipSyncType;
	USoundWaveProcedural* m_soundWave;
	FLipSyncData m_lipSyncData;
	MessageChunkState m_state = MessageChunkState::Idle;

private:
	const FString m_downloadUrl;
	const TFunction<void(const MessageChunkAudioContainer* chunk)> onStateChanged;
	TArray<uint8> m_rawData;

	void DownloadData();
	void ImportData();
	void GenerateLipSync();
	void OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnImportComplete(USoundWaveProcedural* soundWave);
	void OnLipSyncGenComplete(FLipSyncData lipSyncData);
	void UpdateState(MessageChunkState newState);
};
