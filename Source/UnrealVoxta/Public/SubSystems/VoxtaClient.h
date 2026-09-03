// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Subsystems/WorldSubsystem.h"
#include "VoxtaClient.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTranscriptEntryAdded, const FText&, const FText&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnActiveNpcChanged, APawn*);

class UVoxtaStateTreeSubsystem;

/**
 * High-level subsystem for Voxta integration.
 * Provides public-facing API for generic interactions with objects in the level.
 * (e.g. start conversations, register npcs, send messages, etc.)
 *
 * Is persistent within the loaded level.
 *
 * Internally relies on the UVoxtaSubsystem to sync with the server.
 */
UCLASS()
class UNREALVOXTA_API UVoxtaClient : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	FOnTranscriptEntryAdded m_OnTranscriptEntryAdded;
	FOnActiveNpcChanged m_OnCurrentConversableNpcChanged;

	virtual void PostInitialize() override;
	virtual void Deinitialize() override;

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
	UPROPERTY(Transient)
	TWeakObjectPtr<UVoxtaStateTreeSubsystem> m_voxtaSubsystem;
};
