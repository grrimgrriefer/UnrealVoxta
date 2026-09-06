// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAttemptConnectTask.h"
#include "StateTreeExecutionContext.h"
#include "RawAPI/VoxtaApiHandler.h"
#include "SubSystems/VoxtaStateTreeSubsystem.h"

FVoxtaAttemptConnectTask::FVoxtaAttemptConnectTask()
{
	bShouldCallTick = false;
}
const UStruct* FVoxtaAttemptConnectTask::GetInstanceDataType() const
{
	return FInstanceDataType::StaticStruct();
}
EStateTreeRunStatus FVoxtaAttemptConnectTask::EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	FInstanceDataType& instanceData = context.GetInstanceData(*this);
	UVoxtaApiHandler* voxtaSocketHandler = context.GetExternalDataPtr(m_VoxtaApiHandlerHandle);
	ensure(voxtaSocketHandler);

	if (!voxtaSocketHandler)
	{
		UE_LOG(LogTemp, Error, TEXT("[VoxtaConnectTask] Failed to resolve UVoxtaSubsystem."));
		return EStateTreeRunStatus::Failed;
	}

	voxtaSocketHandler->EstablishConnection(instanceData.m_UserConfig.m_VoxtaServerIpv4, instanceData.m_UserConfig.m_VoxtaServerPort);

	return EStateTreeRunStatus::Running;
}
