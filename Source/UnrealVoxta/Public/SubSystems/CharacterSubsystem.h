// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/WeakInterfacePtr.h"
#include "CharacterSubsystem.generated.h"

class IVoxtaClient;
struct FVoxtaCharacterData;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnActiveNpcChanged, APawn*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnChatLoadingProgress, const FText&, float);
DECLARE_MULTICAST_DELEGATE(FOnChatStarted);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTranscriptEntryAdded, const FText&, const FText&);

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
	FOnActiveNpcChanged m_OnCurrentConversableNpcChanged;
	FOnChatLoadingProgress m_OnChatLoadingProgress;
	FOnChatStarted m_OnChatStarted;
	FOnTranscriptEntryAdded m_OnTranscriptEntryAdded;

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

	FDelegateHandle m_chatLoadingHandle;
	FDelegateHandle m_chatStartedHandle;
};
