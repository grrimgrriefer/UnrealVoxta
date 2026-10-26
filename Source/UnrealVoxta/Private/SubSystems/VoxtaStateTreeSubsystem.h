#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "StateTreeExecutionContext.h"
#include "VoxtaRuntimeData.h"
#include "VoxtaUserConfiguration.h"
#include "StateTree/StateTreeContextBinder.h"
#include "Interfaces/VoxtaClient.h"
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
UCLASS()
class UVoxtaStateTreeSubsystem : public UGameInstanceSubsystem, public FTickableGameObject, public IVoxtaClient
{
	GENERATED_BODY()

public:
#pragma region UGameInstanceSubsystem
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
	virtual FOnVoxtaCharactersLoaded& OnCharactersLoaded() override;
	virtual void EnsureConnectionWithServer() override;
	virtual bool TrySendFlowEvent(FGameplayTag tag, bool hasPayload, const FConstStructView& payload) override;
	virtual const TSet<VoxtaClientState>& GetStates() const override;
	virtual const FVoxtaUserConfiguration& GetUserConfiguration() const override;
	virtual TConstArrayView<FVoxtaCharacterData> GetAvailableCharacters() const override;
#pragma endregion

	bool TryMarkNewStateActive(VoxtaClientState voxtaClientState);
	bool TryMarkStateInactive(VoxtaClientState voxtaClientState);
	bool TryBindContextData(UObject* data);
	bool TryUnbindContextData(UObject* data);

	void SetRuntimeData(const FVoxtaRuntimeData& runtimeData);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxta", meta=(RequiredAssetDataTags="Schema=UVoxtaStateTreeSchema"))
	TObjectPtr<UStateTree> m_stateTreeAsset;

private:
	void OnVoxtaConnected();
	void OnVoxtaConnectionError(const FString& error);
	void OnVoxtaDisconnected();
	void OnGameModePostLoginEvent(AGameModeBase* gameMode, APlayerController* newPlayer);

	UPROPERTY()
	FStateTreeInstanceData m_instanceData;
	UPROPERTY()
	FVoxtaUserConfiguration m_voxtaUserConfiguration;
	UPROPERTY()
	TObjectPtr<UVoxtaApiHandler> m_voxtaApiHandler;

	uint32 m_lastFrameNumberWeTicked = INDEX_NONE;
	bool m_isRunning = false;

	StateTreeContextBinder m_contextBinder;
	TSet<VoxtaClientState> m_currentStates;
	FVoxtaRuntimeData m_runtimeData;
};
