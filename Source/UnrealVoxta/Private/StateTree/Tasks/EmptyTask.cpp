// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "EmptyTask.h"

#include "StateTreeExecutionContext.h"
#include "Engine/World.h"

FEmptyTask::FEmptyTask()
{
	bShouldCallTick = false;
}
const UScriptStruct* FEmptyTask::GetInstanceDataType() const
{
	return FInstanceDataType::StaticStruct();
}
EStateTreeRunStatus FEmptyTask::EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	return EStateTreeRunStatus::Running;
}
