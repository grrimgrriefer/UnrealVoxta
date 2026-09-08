// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaTryConnectTask.h"
#include "StateTreeExecutionContext.h"
#include "RawAPI/VoxtaApiHandler.h"
#include "SubSystems/VoxtaStateTreeSubsystem.h"

const UStruct* FVoxtaTryConnectTask::GetInstanceDataType() const
{
	return FInstanceDataType::StaticStruct();
}
EStateTreeRunStatus FVoxtaTryConnectTask::EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	FInstanceDataType& instanceData = context.GetInstanceData(*this);
	UVoxtaApiHandler* voxtaSocketHandler = context.GetExternalDataPtr(m_VoxtaApiHandlerHandle);
	ensure(voxtaSocketHandler);

	if (!voxtaSocketHandler)
	{
		UE_LOG(LogTemp, Error, TEXT("[VoxtaConnectTask] Failed to resolve UVoxtaSubsystem."));
		return EStateTreeRunStatus::Failed;
	}

	voxtaSocketHandler->EstablishConnection(instanceData.m_VoxtaServerIpv4, instanceData.m_VoxtaServerPort);

	return EStateTreeRunStatus::Running;
}
