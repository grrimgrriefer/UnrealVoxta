// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ConversationSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTranscriptEntryAdded, const FText&, const FText&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnActiveNpcChanged, APawn*);

/**
 * High-level subsystem for Voxta integration.
 * Provides simplified API for generic interactions with objects in the level.
 * (e.g. start conversations, register npcs, send messages, etc.)
 *
 * Is persistent within the loaded level.
 *
 * Internally relies on the UVoxtaSubsystem to sync with the server.
 * You can use that one directly if needed, and if you know what you're doing.
 */
UCLASS()
class UNREALVOXTA_API UConversationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	FOnTranscriptEntryAdded m_OnTranscriptEntryAdded;
	FOnActiveNpcChanged m_OnCurrentConversableNpcChanged;

	void RegisterNPC(APawn* npc);
	void UnregisterNPC(APawn* npc);

	void StartConversation(APawn* npc);
	void SubmitMessageFromPlayer(const FText& messageText) const;

	APawn* GetCurrentConversationNpc() const;
	bool IsInOngoingConversation() const;
	APawn* TryGetNearestNPC(const APawn* player) const;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> m_activeNpc = nullptr;
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<APawn>> m_registeredNpcs;
};
