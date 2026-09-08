// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "SubSystems/VoxtaStateTreeSubsystem.h"
#include "IHubConnection.h"
#include "StateTree.h"
#include "VoxtaClientState.h"
#include "VoxtaPluginSettings.h"
#include "RawAPI/VoxtaApiHandler.h"
#include "StateTree/VoxtaStateTreeTags.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "StateTree/Tasks/VoxtaTryConnectTask.h"


#pragma region UGameInstanceSubsystem
void UVoxtaStateTreeSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);

	const UVoxtaPluginSettings* settings = GetDefault<UVoxtaPluginSettings>();
	if (settings && settings->m_stateTreeAsset.IsValid())
	{
		m_stateTreeAsset = Cast<UStateTree>(settings->m_stateTreeAsset.TryLoad());
	}

	m_voxtaApiHandler = NewObject<UVoxtaApiHandler>(this);
	FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &UVoxtaStateTreeSubsystem::OnGameModePostLoginEvent);
}
void UVoxtaStateTreeSubsystem::Deinitialize()
{
	FGameModeEvents::GameModePostLoginEvent.RemoveAll(this);
	if (m_isRunning && IsValid(m_stateTreeAsset))
	{
		FStateTreeExecutionContext context(*this, *m_stateTreeAsset, m_instanceData);
		if (m_contextBinder.SetContextRequirements(context, m_stateTreeAsset, this))
		{
			context.Stop();
		}
		m_isRunning = false;
		UE_LOG(LogTemp, Log, TEXT("%s: StateTree stopped."), *GetNameSafe(this));
	}
	Super::Deinitialize();
}
#pragma endregion


#pragma region FTickableGameObject
void UVoxtaStateTreeSubsystem::Tick(const float deltaTime)
{
	if (m_lastFrameNumberWeTicked == GFrameCounter)
	{
		return;
	}
	m_lastFrameNumberWeTicked = GFrameCounter;

	const UWorld* world = GetWorld();
	if (!world || world->IsPreparingMapChange())
	{
		return;
	}

	ensure(IsValid(m_stateTreeAsset));
	if (m_isRunning && IsValid(m_stateTreeAsset))
	{
		FStateTreeExecutionContext context(*this, *m_stateTreeAsset, m_instanceData);
		if (m_contextBinder.SetContextRequirements(context, m_stateTreeAsset, this))
		{
			context.Tick(deltaTime);
		}
	}
}
ETickableTickType UVoxtaStateTreeSubsystem::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}
TStatId UVoxtaStateTreeSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UVoxtaSubsystem, STATGROUP_Tickables);
}
bool UVoxtaStateTreeSubsystem::IsTickableWhenPaused() const
{
	return true;
}
bool UVoxtaStateTreeSubsystem::IsTickableInEditor() const
{
	return false;
}
bool UVoxtaStateTreeSubsystem::IsTickable() const
{
	return !HasAnyFlags(RF_ClassDefaultObject) && m_isRunning && IsValid(m_stateTreeAsset);
}
#pragma endregion


#pragma region IVoxtaClient
const TSet<VoxtaClientState>& UVoxtaStateTreeSubsystem::GetStates() const
{
	return m_currentStates;
}
const FVoxtaUserConfiguration& UVoxtaStateTreeSubsystem::GetUserConfiguration() const
{
	return m_voxtaUserConfiguration;
}
void UVoxtaStateTreeSubsystem::EnsureConnectionWithServer()
{
	if (m_currentStates.Contains(VoxtaClientState::Authenticated))
	{
		return;
	}

	if (m_currentStates.Contains(VoxtaClientState::AttemptingToConnect) ||
		m_currentStates.Contains(VoxtaClientState::AttemptingToAuthenticate))
	{
		return; // TODO: not sure what to do with additional Ensure calls if the connection fails tbh
	}

	FVoxtaConnectPayload payload;
	payload.m_VoxtaServerIpv4 = m_voxtaUserConfiguration.m_VoxtaServerIpv4;
	payload.m_VoxtaServerPort = m_voxtaUserConfiguration.m_VoxtaServerPort;
	TrySendFlowEvent(TAG_Voxta_Request_Connection, true, FConstStructView::Make(payload));
}
#pragma endregion


bool UVoxtaStateTreeSubsystem::TryMarkNewStateActive(VoxtaClientState voxtaClientState)
{
	bool alreadyActive = false;
	m_currentStates.Emplace(voxtaClientState, &alreadyActive);
	return !alreadyActive;
}
bool UVoxtaStateTreeSubsystem::TryMarkStateInactive(VoxtaClientState voxtaClientState)
{
	return m_currentStates.Remove(voxtaClientState) > 0;
}
bool UVoxtaStateTreeSubsystem::TrySendFlowEvent(const FGameplayTag tag, bool hasPayload, const FConstStructView& payload)
{
	const UWorld* world = GetWorld();

	ensure(world);
	ensure(!world->IsPreparingMapChange());
	ensure(IsValid(m_stateTreeAsset));

	if (m_isRunning && world && world->IsPreparingMapChange() && !IsValid(m_stateTreeAsset))
	{
		FStateTreeExecutionContext context(*this, *m_stateTreeAsset, m_instanceData);
		if (m_contextBinder.SetContextRequirements(context, m_stateTreeAsset, this))
		{
			hasPayload
				? context.SendEvent(tag, payload)
				: context.SendEvent(tag);
			return true;
		}
	}
	return false;
}
bool UVoxtaStateTreeSubsystem::TryBindContextData(UObject* data)
{
	return m_contextBinder.TryBindContextData(data);
}
bool UVoxtaStateTreeSubsystem::TryUnbindContextData(UObject* data)
{
	return m_contextBinder.TryUnbindContextData(data);
}
void UVoxtaStateTreeSubsystem::InitializeInternalRuntimeInfo(FString userName, UObject characterList)
{
	// TODO: store this in a separate component of this subsystem, (runtime data component or something)
}
void UVoxtaStateTreeSubsystem::OnGameModePostLoginEvent(AGameModeBase* gameMode, APlayerController* newPlayer)
{
	if (m_isRunning)
	{
		return;
	}

	ensure(IsValid(m_stateTreeAsset));
	if (!IsValid(m_stateTreeAsset))
	{
		return;
	}

	TryBindContextData(this);
	TryBindContextData(m_voxtaApiHandler);
	FStateTreeExecutionContext context(*this, *m_stateTreeAsset, m_instanceData);
	if (m_contextBinder.SetContextRequirements(context, m_stateTreeAsset, this))
	{
		if (context.Start() == EStateTreeRunStatus::Running)
		{
			m_isRunning = true;
			UE_LOG(LogTemp, Log, TEXT("%s: StateTree started."), *GetNameSafe(this));
		}
	}
}
