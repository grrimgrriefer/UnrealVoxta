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
#include "Audio2FaceRESTHandler.h"
#include "Templates/SharedPointer.h"

enum class MessageChunkState : uint8
{
	Idle,
	Idle_Downloaded,
	Idle_Imported,
	Busy,
	ReadyForPlayback,
	CleanedUp
};

class MessageChunkAudioContainer : public TSharedFromThis<MessageChunkAudioContainer>
{
public:
	MessageChunkAudioContainer(const FString& fullUrl,
		LipSyncType lipSyncType,
		Audio2FaceRESTHandler* A2FRestHandler,
		TFunction<void(const MessageChunkAudioContainer* newState)> callback,
		int id);

	void Continue();
	void CleanupData();
	TArray<uint8> GetRawData();
	template<class T>
	T* GetLipSyncDataPtr() const;

	const int m_index;
	const LipSyncType m_lipSyncType;
	USoundWaveProcedural* m_soundWave;
	MessageChunkState m_state = MessageChunkState::Idle;

	ILipSyncDataBase* m_lipSyncData;

private:
	const FString m_downloadUrl;
	const TFunction<void(const MessageChunkAudioContainer* chunk)> onStateChanged;
	TArray<uint8> m_rawData;
	Audio2FaceRESTHandler* m_A2FRestHandler;

	void DownloadData();
	void ImportData();
	void GenerateLipSync();
	void OnImportComplete(USoundWaveProcedural* soundWave);
	void UpdateState(MessageChunkState newState);
};
