#pragma once

#include "CoreMinimal.h"
#include "SignalRValue.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "StateTreeExecutionContext.h"
#include "VoxtaUserConfiguration.h"
#include "VoxtaSubsystem.generated.h"

class IHubConnection;
class UStateTree;
class AGameModeBase;

/**
 * Low-level subsystem for Voxta integration in the game.
 * Owns the connection socket to the VoxtaServer and the StateTree that dictates and syncs
 * with the VoxtaServer backend.
 *
 * Is persistent across the entire gameinstance.
 *
 * Only use this if you know what you're doing, for a simplified API, see UConversationSubsystem.
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

	void EstablishConnection(const FString& ipv4Address, int port);
	void Disconnect();
	bool TrySend(const FString& message) const;
	const VoxtaUserConfiguration& GetVoxtaUserConfiguration() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTree> m_stateTreeAsset;

private:
	static const FString SEND_MESSAGE_EVENT_NAME;
	static const FString RECEIVE_MESSAGE_EVENT_NAME;

	void OnConnected();
	void OnConnectionError(const FString& String);
	void OnClosed();
	void OnReceivedMessage(const TArray<FSignalRValue>& payload);
	void OnGameModePostLoginEvent(AGameModeBase* gameMode, APlayerController* newPlayer);
	bool TrySendFlowEvent(FGameplayTag tag);

	UPROPERTY()
	FStateTreeInstanceData m_instanceData;

	TSharedPtr<IHubConnection> m_hub;
	VoxtaUserConfiguration m_voxtaUserConfiguration;

	uint32 m_lastFrameNumberWeTicked = INDEX_NONE;
	bool m_isRunning = false;
};
