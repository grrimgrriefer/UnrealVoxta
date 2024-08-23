// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundWaveProcedural.h"
#include "LipSyncType.h"
#include "LipSyncDataBase.h"
#if WITH_OVRLIPSYNC
#include "LipSyncDataOVR.h"
#endif
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
#if WITH_OVRLIPSYNC
	ULipSyncDataOVR* GetLipSyncDataPtr() const;
#endif

	const int m_index;
	const LipSyncType m_lipSyncType;
	USoundWaveProcedural* m_soundWave;
	MessageChunkState m_state = MessageChunkState::Idle;

	ILipSyncDataBase* m_lipSyncData;

private:
	const FString m_downloadUrl;
	const TFunction<void(const MessageChunkAudioContainer* chunk)> onStateChanged;
	TArray<uint8> m_rawData;

	void DownloadData();
	void ImportData();
	void GenerateLipSync();
	void OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnImportComplete(USoundWaveProcedural* soundWave);
	void UpdateState(MessageChunkState newState);
};
