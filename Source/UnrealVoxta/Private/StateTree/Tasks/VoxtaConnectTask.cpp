// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaConnectTask.h"
#include "StateTreeExecutionContext.h"
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
	UVoxtaSubsystem* voxtaSubsystem = context.GetExternalDataPtr(m_VoxtaSocketHandlerHandle);
	ensure(voxtaSubsystem);

	if (!voxtaSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[VoxtaConnectTask] Failed to resolve UVoxtaSubsystem."));
		return EStateTreeRunStatus::Failed;
	}

	const VoxtaUserConfiguration configuration = voxtaSubsystem->GetVoxtaUserConfiguration();
	voxtaSubsystem->EstablishConnection(configuration.m_VoxtaServerIpv4, configuration.m_VoxtaServerPort);

	return EStateTreeRunStatus::Running;
}
void FVoxtaConnectTask::ExitState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	UVoxtaSubsystem* voxtaSubsystem = context.GetExternalDataPtr(m_VoxtaSocketHandlerHandle);
	ensure(voxtaSubsystem);

	if (voxtaSubsystem)
	{
		voxtaSubsystem->Disconnect();
	}
}
