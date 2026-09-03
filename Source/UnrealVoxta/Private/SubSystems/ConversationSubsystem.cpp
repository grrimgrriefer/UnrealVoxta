// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "SubSystems/ConversationSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SubSystems/VoxtaSubsystem.h"

void UConversationSubsystem::PostInitialize()
{
	Super::PostInitialize();
	m_voxtaSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UVoxtaSubsystem>();
	m_voxtaSubsystem->EnsureConnectionWithServer();
}
void UConversationSubsystem::Deinitialize()
{
	Super::Deinitialize();

}
void UConversationSubsystem::RegisterNPC(APawn* npc)
{
	if (IsValid(npc))
	{
		m_registeredNpcs.AddUnique(npc);
	}
}
void UConversationSubsystem::UnregisterNPC(APawn* npc)
{
	if (IsValid(npc))
	{
		m_registeredNpcs.Remove(npc);
	}
}
void UConversationSubsystem::StartConversation(APawn* npc)
{
	if (m_activeNpc.Get() != npc)
	{
		m_activeNpc = npc;
		m_OnCurrentConversableNpcChanged.Broadcast(m_activeNpc.Get());
	}
}
APawn* UConversationSubsystem::GetCurrentConversationNpc() const
{
	return m_activeNpc.Get();
}
bool UConversationSubsystem::IsInOngoingConversation() const
{
	return m_activeNpc.IsValid();
}
APawn* UConversationSubsystem::TryGetNearestNPC(const APawn* player) const
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
void UConversationSubsystem::SubmitMessageFromPlayer(const FText& messageText) const
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
