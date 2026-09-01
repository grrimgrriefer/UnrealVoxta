#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "StateTreeExecutionContext.h"
#include "VoxtaSubsystem.generated.h"

class UStateTree;
class AGameModeBase;

/**
 * Low-level subsystem for Voxta integration in the game.
 * Manages the stateful connection to the VoxtaServer by keeping the
 * internal StateTree in sync with the server.
 *
 * Is persistent across the entire game.
 *
 * Only use this if you know what you're doing, for a simplified API, see UConversationSubsystem.
 */
UCLASS(Abstract, Blueprintable)
class UNREALVOXTA_API UVoxtaSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
#pragma region UGameInstanceSubsystem
	virtual bool ShouldCreateSubsystem(UObject* outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& collection) override;
	virtual void Deinitialize() override;
#pragma endregion

#pragma region FTickableGameObject
	virtual void Tick(float deltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableWhenPaused() const override;
	virtual bool IsTickableInEditor() const override;
	virtual bool IsTickable() const override;
#pragma endregion

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTree> m_stateTreeAsset;

private:
	void OnGameModePostLoginEvent(AGameModeBase* gameMode, APlayerController* newPlayer);

	UPROPERTY()
	FStateTreeInstanceData m_instanceData;

	uint32 m_lastFrameNumberWeTicked = INDEX_NONE;
	bool m_isRunning = false;
};
