// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "SubSystems/CharacterSubsystem.h"
#include "VoxtaClientState.h"
#include "VoxtaGlobals.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/VoxtaCharacter.h"
#include "Interfaces/VoxtaClient.h"
#include "StateTree/VoxtaStateTreeTags.h"
#include "StateTree/Tasks/VoxtaTryStartChatTask.h"
#include "SubSystems/VoxtaStateTreeSubsystem.h"

bool UCharacterSubsystem::ShouldCreateSubsystem(UObject* outer) const
{
	if (!Super::ShouldCreateSubsystem(outer))
	{
		return false;
	}

	const UWorld* world = Cast<UWorld>(outer);
	return world && world->IsGameWorld();
}
void UCharacterSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	m_voxtaClient = IVoxtaClient::Get(GetWorld());
	ensureAlways(m_voxtaClient.IsValid());
	if (m_voxtaClient.IsValid())
	{
		m_voxtaClient->EnsureConnectionWithServer();
	}
}
void UCharacterSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
bool UCharacterSubsystem::TryRegisterNPC(APawn* npc, const FVoxtaCharacterData& voxtaChardata)
{
	ensureAlways(IsValid(npc));
	ensureAlways(!voxtaChardata.m_Id.IsEmpty());

	if (m_registeredNpcs.Contains(npc))
	{
		UE_LOGFMT(LogVoxta, Error, "[{0}] Pawn {1} cannot be registered to {2} as it was already registered to {3}.",
			StaticClass()->GetName(),
			npc->GetName(),
			voxtaChardata.m_Id,
			m_registeredNpcs[npc]);
		return false;
	}
	else
	{
		const TWeakObjectPtr<APawn>* previousPawn = m_registeredNpcs.FindKey(voxtaChardata.m_Id);
		if (previousPawn != nullptr)
		{
			if (!previousPawn->IsValid())
			{
				UE_LOGFMT(LogVoxta, Error, "[{0}] Pawn {1} cannot be registered to {2}. Another pawn has already been registered to it but has been deleted?",
					StaticClass()->GetName(),
					npc->GetName(),
					voxtaChardata.m_Id);
			}
			else
			{
				UE_LOGFMT(LogVoxta, Error, "[{0}] Pawn {1} cannot be registered to {2}. Another pawn has already been registered to it but has been deleted?",
					StaticClass()->GetName(),
					npc->GetName(),
					voxtaChardata.m_Id,
					previousPawn->Get()->GetName());
			}

			return false;
		}
	}

	m_registeredNpcs.Add(npc, voxtaChardata.m_Id);
	if (IVoxtaCharacter* voxtaChar = Cast<IVoxtaCharacter>(npc))
	{
		voxtaChar->OnRegistered(voxtaChardata);
	}

	return true;
}
bool UCharacterSubsystem::IsNpcRegistered(const APawn* npc) const
{
	return m_registeredNpcs.Contains(npc);
}
void UCharacterSubsystem::UnregisterNPC(APawn* npc)
{
	ensureAlways(IsValid(npc));

	if (m_registeredNpcs.Remove(npc) > 0)
	{
		if (IVoxtaCharacter* voxtaChar = Cast<IVoxtaCharacter>(npc))
		{
			voxtaChar->OnUnregistered();
		}
	}
	else
	{
		UE_LOGFMT(LogVoxta, Warning, "[{0}] Pawn {1} cannot be unregistered, as it was not registered?", StaticClass()->GetName(), npc->GetName());
	}
}
bool UCharacterSubsystem::TryStartConversation(APawn* npc)
{
	ensureAlways(IsValid(npc));

	if (m_activeNpc.Get() == npc)
	{
		UE_LOGFMT(LogVoxta, Warning, "[{0}] Pawn {1} cannot start conversation, as it was already active?", StaticClass()->GetName(), npc->GetName());
		return false;
	}

	auto characterId = m_registeredNpcs.Find(npc);
	if (characterId == nullptr)
	{
		UE_LOGFMT(LogVoxta, Warning, "[{0}] Pawn {1} cannot start conversation, as it wasn't registered first.", StaticClass()->GetName(), npc->GetName());
		return false;
	}

	m_activeNpc = npc;

	FVoxtaTryStartChatPayload payload;
	payload.m_ChatCharacter = *characterId;
	m_voxtaClient->TrySendFlowEvent(TAG_Voxta_Request_StartChat, true, FConstStructView::Make(payload));

	// TOOD: this probably should get fired by that Task that is entered/left that handles the actual chat
	// or do we want to trigger this while the chat is still being started in the background? Not sure.
	m_OnCurrentConversableNpcChanged.Broadcast(m_activeNpc.Get());

	return true;
}
void UCharacterSubsystem::StopConversation()
{
	m_activeNpc = nullptr;
	m_OnCurrentConversableNpcChanged.Broadcast(nullptr);
}
APawn* UCharacterSubsystem::GetCurrentConversationNpc() const
{
	return m_activeNpc.Get();
}
bool UCharacterSubsystem::IsInOngoingConversation() const
{
	return m_activeNpc.IsValid();
}
APawn* UCharacterSubsystem::TryGetNearestNPC(const APawn* player) const
{
	if (!IsValid(player))
	{
		return nullptr;
	}

	APawn* nearestNpc = nullptr;
	float minDistanceSq = FLT_MAX;
	const FVector playerLoc = player->GetActorLocation();

	for (const auto& [weakNpc, npcString] : m_registeredNpcs)
	{
		if (APawn* npc = weakNpc.Get())
		{
			const float distSq = FVector::DistSquared(playerLoc, npc->GetActorLocation());
			if (distSq < minDistanceSq)
			{
				minDistanceSq = distSq;
				nearestNpc = npc;
			}
		}
	}

	return nearestNpc;
}
void UCharacterSubsystem::SubmitMessageFromPlayer(const FText& messageText)
{
	if (messageText.IsEmptyOrWhitespace())
	{
		return;
	}

	// TODO: Get player name from Voxta
	const FText playerSpeaker = NSLOCTEXT("Conversation", "YouSpeaker", "You");
	m_OnTranscriptEntryAdded.Broadcast(playerSpeaker, messageText);

	// TODO: Forward input to Voxta
}
