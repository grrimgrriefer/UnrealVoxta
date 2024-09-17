// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAudioPlayback.h"
#include "ChatMessage.h"
#include "VoxtaClient.h"
#include "Audio2FacePlaybackHandler.h"
#include "Internals/MessageChunkAudioContainer.h"
#if WITH_OVRLIPSYNC
#include "OVRLipSyncPlaybackActorComponent.h"
#include "LipSyncDataOVR.h"
#endif
#include "BaseCharData.h"
#include "Sound/SoundWaveProcedural.h"
#include "Logging/StructuredLog.h"

void UVoxtaAudioPlayback::Initialize(const FString& characterId)
{
	m_characterId = characterId;
	m_clientReference = GetWorld()->GetGameInstance()->GetSubsystem<UVoxtaClient>();
#if WITH_OVRLIPSYNC
	if (m_lipSyncType == LipSyncType::OVRLipSync)
	{
		m_lipSyncHandler = GetOwner()->FindComponentByClass<UOVRLipSyncPlaybackActorComponent>();
	}
#endif

	m_hostAddress = m_clientReference->GetServerAddress();
	m_hostPort = m_clientReference->GetServerPort();
	m_charMessageAddedHandle = m_clientReference->VoxtaClientCharMessageAddedEventNative.AddUObject(
		this, &UVoxtaAudioPlayback::PlaybackMessage);

	UE_LOGFMT(VoxtaLog, Log, "Initialized audioplayback for characterId {0}. Audio will be downloaded from: {1}",
		characterId, FString::Format(*FString(TEXT("http://{0}:{1}/")), { m_hostAddress, m_hostPort }));
}

void UVoxtaAudioPlayback::MarkCustomPlaybackComplete(const FGuid& guid)
{
	const FGuid expectedGuid = m_orderedAudio[m_currentAudioClipIndex]->GetLipSyncData<ILipSyncDataBase>()->GetGuid();
	if (expectedGuid != guid)
	{
		UE_LOGFMT(VoxtaLog, Error, "Custom LipSync does not support playing audiochunks out of order. "
			"Was expecting id: {0} received id: {1}", expectedGuid.ToString(), guid.ToString());
		return;
	}

	m_internalState = AudioPlaybackInternalState::Idle;
	UE_LOGFMT(VoxtaLog, Log, "Playback of audioClip with guid: {0} is markd as complete, continueing Voxta logic.",
		guid.ToString());

	MarkAudioChunkPlaybackCompleteInternal();
}

void UVoxtaAudioPlayback::BeginPlay()
{
	Super::BeginPlay();
	m_playbackFinishedHandle = OnAudioFinishedNative.AddUObject(this, &UVoxtaAudioPlayback::OnAudioPlaybackFinished);
	if (m_lipSyncType == LipSyncType::Audio2Face)
	{
		m_lipSyncHandler = NewObject<UAudio2FacePlaybackHandler>(this);
		Cast<UAudio2FacePlaybackHandler>(m_lipSyncHandler)->Initialize(this);

		UE_LOGFMT(VoxtaLog, Log, "Created Audio2Face lipsync handler for characterId: {0}.", m_characterId);
	}
}

void UVoxtaAudioPlayback::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	Super::EndPlay(endPlayReason);
	if (endPlayReason != EEndPlayReason::Quit && endPlayReason != EEndPlayReason::EndPlayInEditor)
	{
		UE_LOGFMT(VoxtaLog, Warning, "Removed audioplayback for character with id: {0} due to EndPlay with reason {1}.",
			m_characterId, UEnum::GetValueAsString(endPlayReason));
	}

	OnAudioFinishedNative.Remove(m_playbackFinishedHandle);

	if (m_clientReference)
	{
		m_clientReference->VoxtaClientCharMessageAddedEventNative.Remove(m_charMessageAddedHandle);
	}
	if (m_lipSyncType == LipSyncType::Audio2Face && m_lipSyncHandler)
	{
		Cast<UAudio2FacePlaybackHandler>(m_lipSyncHandler)->Stop();
	}
#if WITH_OVRLIPSYNC
	if (m_lipSyncType == LipSyncType::OVRLipSync && m_lipSyncHandler)
	{
		Cast<UOVRLipSyncPlaybackActorComponent>(m_lipSyncHandler)->Stop();
	}
#endif
}

void UVoxtaAudioPlayback::GetA2FCurveWeightsPreUpdate(TArray<float>& targetArrayRef)
{
	if (m_lipSyncType == LipSyncType::Audio2Face && m_lipSyncHandler)
	{
		Cast<UAudio2FacePlaybackHandler>(m_lipSyncHandler)->GetA2FCurveWeights(targetArrayRef);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Only ask for A2F CurveWeights if Audio2Face is enabled. "
			"Character with id: {0} is currently marked as using: {1} for lipsync.",
			m_characterId, UEnum::GetValueAsString(m_lipSyncType));
	}
}

void UVoxtaAudioPlayback::PlaybackMessage(const FBaseCharData& sender, const FChatMessage& message)
{
	// Listener gets invoked for all messages, safe to ignore the ones for other characters
	if (sender.GetId() == m_characterId)
	{
		Cleanup();
		m_currentlyPlayingMessageId = message.GetMessageId();

		for (int i = 0; i < message.m_audioUrls.Num(); i++)
		{
			m_orderedAudio.Add(MakeShared<MessageChunkAudioContainer>(
				FString::Format(*FString(TEXT("http://{0}:{1}{2}")), { m_hostAddress, m_hostPort, message.m_audioUrls[i] }),
				m_lipSyncType,
				m_clientReference->GetA2FHandler(),
				[&] (const MessageChunkAudioContainer* chunk) { OnChunkStateChange(chunk); },
				i));
		}
		m_internalState = AudioPlaybackInternalState::Idle;
		m_orderedAudio[0]->Continue();

		UE_LOGFMT(VoxtaLog, Log, "Started playback for messageId: {0} of SenderId: {1}. Full message contains {2} "
			"audio chunks that will be played in sequence.",
			m_currentlyPlayingMessageId, m_characterId, m_orderedAudio.Num());
	}
}

void UVoxtaAudioPlayback::PlayCurrentAudioChunkIfAvailable()
{
	if (m_internalState != AudioPlaybackInternalState::Idle)
	{
		UE_LOGFMT(VoxtaLog, Error, "Tried to play an audiochunk but playback is currently not Idle.");
		return;
	}
	MessageChunkAudioContainer* currentClip = m_orderedAudio[m_currentAudioClipIndex].Get();
	if (currentClip->GetCurrentState() == MessageChunkState::ReadyForPlayback)
	{
		m_internalState = AudioPlaybackInternalState::Playing;

		if (currentClip->LIP_SYNC_TYPE != LipSyncType::Custom)
		{
			UE_LOGFMT(VoxtaLog, Log, "Starting playback of audio chunk index: {0}, with lipsync type: {1}.",
				currentClip->INDEX, UEnum::GetValueAsString(currentClip->LIP_SYNC_TYPE));
		}

		switch (currentClip->LIP_SYNC_TYPE)
		{
			case LipSyncType::None:
				SetSound(currentClip->GetSoundWave());
				Play();
				break;
			case LipSyncType::Custom:
			{
				TArray<uint8> rawData = currentClip->GetRawAudioData();
				USoundWaveProcedural* soundWave = currentClip->GetSoundWave();
				FGuid guid = currentClip->GetLipSyncData<ILipSyncDataBase>()->GetGuid();

				UE_LOGFMT(VoxtaLog, Log, "Broadcasting that audio chunk with guid: {0} is ready for playback with "
					"custom lipsync. Voxta logic will wait to continue until it is marked as finished "
					"via MarkCustomPlaybackComplete", guid);

				VoxtaMessageAudioChunkReadyForCustomPlaybackEventNative.Broadcast(rawData, soundWave, guid);
				VoxtaMessageAudioChunkReadyForCustomPlaybackEvent.Broadcast(rawData, soundWave, guid);
				break;
			}
			case LipSyncType::OVRLipSync:
#if WITH_OVRLIPSYNC
				SetSound(currentClip->GetSoundWave());
				Cast<UOVRLipSyncPlaybackActorComponent>(m_lipSyncHandler)->Start(
					this, currentClip->GetLipSyncData<ULipSyncDataOVR>()->GetOvrLipSyncData());
#else
				UE_LOGFMT(VoxtaLog, Error, "OvrLipSync was selected, but the module is not present in the project.");
#endif
				break;
			case LipSyncType::Audio2Face:
				SetSound(currentClip->GetSoundWave());
				Cast<UAudio2FacePlaybackHandler>(m_lipSyncHandler)->Play(currentClip->GetLipSyncData<ULipSyncDataA2F>());
				break;
			default:
				UE_LOGFMT(VoxtaLog, Error, "Unsupported selection for LipSync, this should never happen. Aborting playback");
				return;
		}
	}
}

void UVoxtaAudioPlayback::OnAudioPlaybackFinished(UAudioComponent* component)
{
	if (m_lipSyncType == LipSyncType::Custom)
	{
		// We cannot rely on callbacks, as the user might play audio through another provider.
		return;
	}

	UE_LOGFMT(VoxtaLog, Log, "Automatic playback of audio chunk index: {0} is complete.", m_currentAudioClipIndex);

	m_internalState = AudioPlaybackInternalState::Idle;
	MarkAudioChunkPlaybackCompleteInternal();
}

void UVoxtaAudioPlayback::MarkAudioChunkPlaybackCompleteInternal()
{
	m_orderedAudio[m_currentAudioClipIndex]->CleanupData();
	m_currentAudioClipIndex += 1;
	if (m_currentAudioClipIndex < m_orderedAudio.Num())
	{
		PlayCurrentAudioChunkIfAvailable();
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Log, "Playback of all audiochunks for message with id: {0} is finished.",
			m_currentlyPlayingMessageId);

		VoxtaMessageAudioPlaybackFinishedEventNative.Broadcast(m_currentlyPlayingMessageId);
		VoxtaMessageAudioPlaybackFinishedEvent.Broadcast(m_currentlyPlayingMessageId);
		Cleanup();
	}
}

void UVoxtaAudioPlayback::OnChunkStateChange(const MessageChunkAudioContainer* chunk)
{
	if (m_internalState == AudioPlaybackInternalState::Done)
	{
		UE_LOGFMT(VoxtaLog, Error, "Audio playback was marked as finished, but a chunk was still underway, discarding.");
		return;
	}

	if (m_orderedAudio[chunk->INDEX]->GetCurrentState() == MessageChunkState::ReadyForPlayback && m_internalState == AudioPlaybackInternalState::Idle)
	{
		PlayCurrentAudioChunkIfAvailable();
	}
	m_orderedAudio[chunk->INDEX]->Continue();
	if (chunk->INDEX + 1 < m_orderedAudio.Num())
	{
		m_orderedAudio[chunk->INDEX + 1]->Continue();
	}
}

void UVoxtaAudioPlayback::Cleanup()
{
	UE_LOGFMT(VoxtaLog, Log, "Cleaning up all memory usage for audio related to audio for message with id: {0}.",
		m_currentlyPlayingMessageId);

	m_currentlyPlayingMessageId = FString(TEXT("NULL"));
	m_currentAudioClipIndex = 0;
	m_internalState = AudioPlaybackInternalState::Done;
	for (TSharedPtr<MessageChunkAudioContainer> audioChunk : m_orderedAudio)
	{
		audioChunk->CleanupData();
		audioChunk = nullptr;
	}
	m_orderedAudio.Empty();
}