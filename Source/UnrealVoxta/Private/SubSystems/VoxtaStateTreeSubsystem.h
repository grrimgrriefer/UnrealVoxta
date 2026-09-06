#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "StateTreeExecutionContext.h"
#include "VoxtaUserConfiguration.h"
#include "SubSystems/VoxtaClient.h"
#include "VoxtaStateTreeSubsystem.generated.h"

class UStateTree;
class AGameModeBase;
class UVoxtaApiHandler;

/**
 * Holds the persistent StateTree that manages the Voxta integration.
 * Controls the high-level flow of the game by dictating the major gameplay states.
 *
 * Is persistent across the entire gameinstance.
 */
UCLASS(Abstract, Blueprintable)
class UVoxtaStateTreeSubsystem : public UGameInstanceSubsystem, public FTickableGameObject, public IVoxtaClient
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

#pragma region IVoxtaClient
	virtual const FVoxtaUserConfiguration& GetVoxtaUserConfiguration() const override;
	virtual void EnsureConnectionWithServer() const override;
#pragma endregion

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxta", meta=(RequiredAssetDataTags="Schema=UVoxtaTreeSchema"))
	TObjectPtr<UStateTree> m_stateTreeAsset;

private:
	void OnGameModePostLoginEvent(AGameModeBase* gameMode, APlayerController* newPlayer);
	bool TrySendFlowEvent(FGameplayTag tag);

	UPROPERTY()
	FStateTreeInstanceData m_instanceData;
	UPROPERTY()
	FVoxtaUserConfiguration m_voxtaUserConfiguration;
	UPROPERTY()
	TObjectPtr<UVoxtaApiHandler> m_voxtaApiHandler;

	uint32 m_lastFrameNumberWeTicked = INDEX_NONE;
	bool m_isRunning = false;
};
