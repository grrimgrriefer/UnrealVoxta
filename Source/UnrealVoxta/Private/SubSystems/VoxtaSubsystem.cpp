// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "SubSystems/VoxtaSubsystem.h"
#include "StateTree.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"


#pragma region UGameInstanceSubsystem
bool UVoxtaSubsystem::ShouldCreateSubsystem(UObject* outer) const
{
	return GetClass() != StaticClass();
}
void UVoxtaSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);
	FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &UVoxtaSubsystem::OnGameModePostLoginEvent);
}
void UVoxtaSubsystem::Deinitialize()
{
	FGameModeEvents::GameModePostLoginEvent.RemoveAll(this);
	if (m_isRunning && IsValid(m_stateTreeAsset))
	{
		FStateTreeExecutionContext context(*this, *m_stateTreeAsset, m_instanceData);
		context.Stop();

		m_isRunning = false;
		UE_LOG(LogTemp, Log, TEXT("%s: VoxtaStateTree stopped."), *GetNameSafe(this));
	}
	Super::Deinitialize();
}
#pragma endregion


#pragma region FTickableGameObject
void UVoxtaSubsystem::Tick(const float deltaTime)
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
		context.Tick(deltaTime);
	}
}
ETickableTickType UVoxtaSubsystem::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}
TStatId UVoxtaSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UVoxtaSubsystem, STATGROUP_Tickables);
}
bool UVoxtaSubsystem::IsTickableWhenPaused() const
{
	return true;
}
bool UVoxtaSubsystem::IsTickableInEditor() const
{
	return false;
}
bool UVoxtaSubsystem::IsTickable() const
{
	return !HasAnyFlags(RF_ClassDefaultObject) && m_isRunning && IsValid(m_stateTreeAsset);
}
#pragma endregion


void UVoxtaSubsystem::OnGameModePostLoginEvent(AGameModeBase* gameMode, APlayerController* newPlayer)
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

	FStateTreeExecutionContext context(*this, *m_stateTreeAsset, m_instanceData);
	if (context.Start() == EStateTreeRunStatus::Running)
	{
		m_isRunning = true;
		UE_LOG(LogTemp, Log, TEXT("%s: VoxtaStateTree started."), *GetNameSafe(this));
	}
}
