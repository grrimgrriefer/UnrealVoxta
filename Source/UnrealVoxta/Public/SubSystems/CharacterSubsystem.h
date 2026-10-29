// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/WeakInterfacePtr.h"
#include "CharacterSubsystem.generated.h"

class IVoxtaClient;
struct FVoxtaCharacterData;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTranscriptEntryAdded, const FText&, const FText&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnActiveNpcChanged, APawn*);

/**
 * High-level subsystem for Voxta integration.
 * Provides general utility regarding the characters present in the level and their high-level behavior.
 * Also handles the binding of the in-game characters to the Voxta personas
 *
 * Is persistent within the loaded level.
 */
UCLASS()
class UNREALVOXTA_API UCharacterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	FOnTranscriptEntryAdded m_OnTranscriptEntryAdded;
	FOnActiveNpcChanged m_OnCurrentConversableNpcChanged;

	virtual bool ShouldCreateSubsystem(UObject* outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	bool TryRegisterNPC(APawn* npc, const FVoxtaCharacterData& voxtaChardata);
	bool IsNpcRegistered(const APawn* conversableNPC) const;
	void UnregisterNPC(APawn* npc);

	bool TryStartConversation(APawn* npc);
	void StopConversation();
	void SubmitMessageFromPlayer(const FText& messageText);

	APawn* GetCurrentConversationNpc() const;
	bool IsInOngoingConversation() const;
	APawn* TryGetNearestNPC(const APawn* player) const;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> m_activeNpc = nullptr;
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<APawn>, FString> m_registeredNpcs;

	TWeakInterfacePtr<IVoxtaClient> m_voxtaClient;
};
