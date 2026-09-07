// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaEmptyTask.h"
#include "StateTreeExecutionContext.h"
#include "Engine/World.h"

const UScriptStruct* FVoxtaEmptyTask::GetInstanceDataType() const
{
	return FInstanceDataType::StaticStruct();
}
VoxtaClientState FVoxtaEmptyTask::GetStateForTask(FStateTreeExecutionContext& context) const
{
	const FInstanceDataType& instanceData = context.GetInstanceData(*this);
	return instanceData.m_ClientStateForThisTask;
}
