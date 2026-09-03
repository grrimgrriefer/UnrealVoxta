#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "StateTreeExecutionContext.h"
#include "VoxtaUserConfiguration.h"
#include "VoxtaSocketHandler.h"
#include "VoxtaSubsystem.generated.h"

class UStateTree;
class AGameModeBase;

/**
 * Low-level subsystem for Voxta integration in the game.
 * Owns and controls the internal StateTree which dictates the communication between the game
 * and the VoxtaServer.
 *
 * Is persistent across the entire gameinstance.
 */
UCLASS(Abstract, Blueprintable)
class UNREALVOXTA_API UVoxtaSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	static const FName CLIENT_NAME;
	static const FName CLIENT_VERSION;

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

	const VoxtaUserConfiguration& GetVoxtaUserConfiguration() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxta", meta=(RequiredAssetDataTags="Schema=UVoxtaTreeSchema"))
	TObjectPtr<UStateTree> m_stateTreeAsset;

private:
	void OnGameModePostLoginEvent(AGameModeBase* gameMode, APlayerController* newPlayer);
	bool TrySendFlowEvent(FGameplayTag tag);

	UPROPERTY()
	FStateTreeInstanceData m_instanceData;
	UPROPERTY()
	UVoxtaSocketHandler m_voxtaSocketHandler;

	VoxtaUserConfiguration m_voxtaUserConfiguration;

	uint32 m_lastFrameNumberWeTicked = INDEX_NONE;
	bool m_isRunning = false;
};
