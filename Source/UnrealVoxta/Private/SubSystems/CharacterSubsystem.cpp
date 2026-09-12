// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "SubSystems/CharacterSubsystem.h"
#include "VoxtaClientState.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
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
void UCharacterSubsystem::RegisterNPC(APawn* npc)
{
	if (IsValid(npc))
	{
		m_registeredNpcs.AddUnique(npc);
	}
}
void UCharacterSubsystem::UnregisterNPC(APawn* npc)
{
	if (IsValid(npc))
	{
		m_registeredNpcs.Remove(npc);
	}
}
void UCharacterSubsystem::StartConversation(APawn* npc)
{
	if (m_activeNpc.Get() != npc)
	{
		m_activeNpc = npc;
		m_OnCurrentConversableNpcChanged.Broadcast(m_activeNpc.Get());
	}
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

	for (const TWeakObjectPtr<APawn>& weakNpc : m_registeredNpcs)
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
