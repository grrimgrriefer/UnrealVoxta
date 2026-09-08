// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaBaseTask.h"
#include "StateTreeExecutionContext.h"
#include "VoxtaClientState.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "SubSystems/VoxtaStateTreeSubsystem.h"

FVoxtaBaseTask::FVoxtaBaseTask()
{
	bShouldCallTick = true;
}
EStateTreeRunStatus FVoxtaBaseTask::EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	UVoxtaStateTreeSubsystem* subsystem = GetSubsystem(context);
	return IsValid(subsystem) && subsystem->TryMarkNewStateActive(GetStateForTask(context))
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Failed;
}
void FVoxtaBaseTask::ExitState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	UVoxtaStateTreeSubsystem* subsystem = GetSubsystem(context);
	if (IsValid(subsystem))
	{
		subsystem->TryMarkStateInactive(GetStateForTask(context));
	}
}
VoxtaClientState FVoxtaBaseTask::GetStateForTask(FStateTreeExecutionContext& context) const
{
	return VoxtaClientState::Invalid;
}
UVoxtaStateTreeSubsystem* FVoxtaBaseTask::GetSubsystem(FStateTreeExecutionContext& context) const
{
	auto world = context.GetWorld();
	ensureAlways(world);
	if (!IsValid(world)) return nullptr;
	auto gameInstance = world->GetGameInstance();
	ensureAlways(gameInstance);
	if (!IsValid(gameInstance)) return nullptr;
	auto subsystem = gameInstance->GetSubsystem<UVoxtaStateTreeSubsystem>();
	ensureAlways(subsystem);
	return subsystem;
}
