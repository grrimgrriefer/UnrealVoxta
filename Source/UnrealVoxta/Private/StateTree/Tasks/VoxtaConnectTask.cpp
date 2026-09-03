// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaConnectTask.h"
#include "StateTreeExecutionContext.h"
#include "VoxtaSocketHandler.h"
#include "SubSystems/VoxtaSubsystem.h"

FVoxtaConnectTask::FVoxtaConnectTask()
{
	bShouldCallTick = false;
}
const UStruct* FVoxtaConnectTask::GetInstanceDataType() const
{
	return FInstanceDataType::StaticStruct();
}
EStateTreeRunStatus FVoxtaConnectTask::EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	FInstanceDataType& instanceData = context.GetInstanceData(*this);
	UVoxtaSocketHandler* voxtaSocketHandler = context.GetExternalDataPtr(m_VoxtaSocketHandlerHandle);
	ensure(voxtaSocketHandler);

	if (!voxtaSocketHandler)
	{
		UE_LOG(LogTemp, Error, TEXT("[VoxtaConnectTask] Failed to resolve UVoxtaSubsystem."));
		return EStateTreeRunStatus::Failed;
	}

	const VoxtaUserConfiguration configuration = VoxtaUserConfiguration(); // TODO: Get configuration out of the event payload
	voxtaSocketHandler->EstablishConnection(configuration.m_VoxtaServerIpv4, configuration.m_VoxtaServerPort);

	return EStateTreeRunStatus::Running;
}
void FVoxtaConnectTask::ExitState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	UVoxtaSocketHandler* voxtaSocketHandler = context.GetExternalDataPtr(m_VoxtaSocketHandlerHandle);
	ensure(voxtaSocketHandler);

	if (voxtaSocketHandler)
	{
		voxtaSocketHandler->Disconnect();
	}
}
