// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaBaseTask.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "VoxtaClientState.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "SubSystems/VoxtaStateTreeSubsystem.h"

FVoxtaBaseTask::FVoxtaBaseTask()
{
	bShouldCallTick = true;
}
bool FVoxtaBaseTask::Link(FStateTreeLinker& linker)
{
	linker.LinkExternalData(m_VoxtaStateTreeSubsystemHandle);
	return true;
}
EStateTreeRunStatus FVoxtaBaseTask::EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	UVoxtaStateTreeSubsystem* subsystem = context.GetExternalDataPtr(m_VoxtaStateTreeSubsystemHandle);
	ensureAlways(subsystem);
	return IsValid(subsystem) && subsystem->TryMarkNewStateActive(GetStateForTask(context))
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Failed;
}
void FVoxtaBaseTask::ExitState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	UVoxtaStateTreeSubsystem* subsystem = context.GetExternalDataPtr(m_VoxtaStateTreeSubsystemHandle);
	ensureAlways(subsystem);
	if (IsValid(subsystem))
	{
		subsystem->TryMarkStateInactive(GetStateForTask(context));
	}
}
VoxtaClientState FVoxtaBaseTask::GetStateForTask(FStateTreeExecutionContext& context) const
{
	return VoxtaClientState::Invalid;
}
