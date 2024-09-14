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
#include "CharDataBase.h"
#include "Sound/SoundWaveProcedural.h"
#include "Logging/StructuredLog.h"

void UVoxtaAudioPlayback::InitializeAudioPlayback(const FString& characterId)
{
	m_characterId = characterId;
	m_clientReference = GetWorld()->GetGameInstance()->GetSubsystem<UVoxtaClient>();
	m_charMessageAddedHandle = m_clientReference->VoxtaClientCharMessageAddedEventNative.AddUObject(this, &UVoxtaAudioPlayback::PlaybackMessage);

	m_hostAddress = m_clientReference->GetServerAddress();
	m_hostPort = m_clientReference->GetServerPort();

#if WITH_OVRLIPSYNC
	m_ovrLipSync = GetOwner()->FindComponentByClass<UOVRLipSyncPlaybackActorComponent>();
#endif
}

void UVoxtaAudioPlayback::MarkCustomPlaybackComplete(const FGuid& guid)
{
	if (m_orderedAudio[m_currentAudioClip]->GetLipSyncData<ILipSyncDataBase>()->GetGuid() != guid)
	{
		UE_LOGFMT(VoxtaLog, Error, "Custom LipSync does not support playing audiochunks out of order.");
		return;
	}
	m_internalState = InternalState::Idle;
	MarkAudioChunkPlaybackCompleteInternal();
}

void UVoxtaAudioPlayback::BeginPlay()
{
	Super::BeginPlay();
	m_playbackFinishedHandle = OnAudioFinishedNative.AddUObject(this, &UVoxtaAudioPlayback::OnAudioPlaybackFinished);
	if (m_lipSyncType == LipSyncType::Audio2Face)
	{
		m_audio2FacePlaybackHandler = NewObject<UAudio2FacePlaybackHandler>();
	}
}

void UVoxtaAudioPlayback::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	Super::EndPlay(endPlayReason);
	OnAudioFinishedNative.Remove(m_playbackFinishedHandle);

	if (m_clientReference)
	{
		m_clientReference->VoxtaClientCharMessageAddedEventNative.Remove(m_charMessageAddedHandle);
	}
	if (m_audio2FacePlaybackHandler != nullptr)
	{
		m_audio2FacePlaybackHandler->Stop();
	}
#if WITH_OVRLIPSYNC
	if (m_lipSyncType == LipSyncType::OVRLipSync)
	{
		Cast<UOVRLipSyncPlaybackActorComponent>(m_ovrLipSync)->Stop();
	}
#endif
}

void UVoxtaAudioPlayback::GetA2FCurveWeights(TArray<float>& targetArrayRef)
{
	if (m_audio2FacePlaybackHandler != nullptr)
	{
		m_audio2FacePlaybackHandler->GetA2FCurveWeights(targetArrayRef);
	}
}

void UVoxtaAudioPlayback::PlaybackMessage(const FCharDataBase& sender, const FChatMessage& message)
{
	if (sender.GetId() == m_characterId)
	{
		Cleanup();
		m_messageId = message.GetMessageId();

		for (int i = 0; i < message.m_audioUrls.Num(); i++)
		{
			m_orderedAudio.Add(MakeShared<MessageChunkAudioContainer>(
				FString::Format(*FString(TEXT("http://{0}:{1}{2}")), { m_hostAddress, m_hostPort, message.m_audioUrls[i] }),
				m_lipSyncType,
				m_clientReference->GetA2FHandler(),
				[&] (const MessageChunkAudioContainer* chunk) { OnChunkStateChange(chunk); },
				i));
		}
		m_internalState = InternalState::Idle;
		m_orderedAudio[0]->Continue();
	}
}

void UVoxtaAudioPlayback::TryPlayCurrentAudioChunk()
{
	if (m_internalState != InternalState::Idle)
	{
		UE_LOGFMT(VoxtaLog, Error, "Tried to play an audiochunk but playback was not Idle, this should never happen.");
		return;
	}
	if (m_orderedAudio[m_currentAudioClip]->GetCurrentState() == MessageChunkState::ReadyForPlayback)
	{
		// TODO: move this to somewhere else if things get too complicated
		m_internalState = InternalState::Playing;

		switch (m_orderedAudio[m_currentAudioClip]->LIP_SYNC_TYPE)
		{
			case LipSyncType::None:
				SetSound(m_orderedAudio[m_currentAudioClip]->GetSoundWave());
				Play();
				break;
			case LipSyncType::Custom:
				VoxtaMessageAudioChunkReadyForCustomPlaybackEvent.Broadcast(
					m_orderedAudio[m_currentAudioClip]->GetRawAudioData(),
					m_orderedAudio[m_currentAudioClip]->GetSoundWave(),
					m_orderedAudio[m_currentAudioClip]->GetLipSyncData<ILipSyncDataBase>()->GetGuid());
				break;
			case LipSyncType::OVRLipSync:
#if WITH_OVRLIPSYNC
				SetSound(m_orderedAudio[m_currentAudioClip]->GetSoundWave());
				Cast<UOVRLipSyncPlaybackActorComponent>(m_ovrLipSync)->Start(this, m_orderedAudio[m_currentAudioClip]->GetLipSyncData<ULipSyncDataOVR>()->GetOvrLipSyncData());
#else
				UE_LOGFMT(VoxtaLog, Error, "OvrLipSync was selected, but the module is not present in the project.");
#endif
				break;
			case LipSyncType::Audio2Face:
				SetSound(m_orderedAudio[m_currentAudioClip]->GetSoundWave());
				m_audio2FacePlaybackHandler->Play(this, m_orderedAudio[m_currentAudioClip]->GetLipSyncData<ULipSyncDataA2F>());
				break;
			default:
				UE_LOGFMT(VoxtaLog, Error, "Unsupported selection for LipSync, this should never happen.");
				break;
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
	m_internalState = InternalState::Idle;
	MarkAudioChunkPlaybackCompleteInternal();
}

void UVoxtaAudioPlayback::MarkAudioChunkPlaybackCompleteInternal()
{
	m_orderedAudio[m_currentAudioClip]->CleanupData();
	m_currentAudioClip += 1;
	if (m_currentAudioClip < m_orderedAudio.Num())
	{
		TryPlayCurrentAudioChunk();
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Log, "Playback finished.");
		VoxtaMessageAudioPlaybackFinishedEvent.Broadcast(m_messageId);
		Cleanup();
	}
}

void UVoxtaAudioPlayback::OnChunkStateChange(const MessageChunkAudioContainer* chunk)
{
	if (m_internalState == InternalState::Done)
	{
		UE_LOGFMT(VoxtaLog, Error, "Audio playback was marked as finished, but a chunk was still underway, discarding.");
		return;
	}

	if (m_orderedAudio[chunk->INDEX]->GetCurrentState() == MessageChunkState::ReadyForPlayback && m_internalState == InternalState::Idle)
	{
		TryPlayCurrentAudioChunk();
	}
	m_orderedAudio[chunk->INDEX]->Continue();
	if (chunk->INDEX + 1 < m_orderedAudio.Num())
	{
		m_orderedAudio[chunk->INDEX + 1]->Continue();
	}
}

void UVoxtaAudioPlayback::Cleanup()
{
	m_messageId.Empty();
	m_currentAudioClip = 0;
	m_internalState = InternalState::Done;
	for (TSharedPtr<MessageChunkAudioContainer> audioChunk : m_orderedAudio)
	{
		audioChunk->CleanupData(); // Should not be necessary, but can't hurt
		audioChunk = nullptr;
	}
	m_orderedAudio.Empty();
}