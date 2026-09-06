// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "SubSystems/CharacterSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SubSystems/VoxtaStateTreeSubsystem.h"

void UCharacterSubsystem::PostInitialize()
{
	Super::PostInitialize();
	m_voxtaSubsystem = IVoxtaClient::Get(GetWorld());
	m_voxtaSubsystem->EnsureConnectionWithServer();
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
void UCharacterSubsystem::SubmitMessageFromPlayer(const FText& messageText) const
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
