// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaConnectTask.h"
#include "StateTreeExecutionContext.h"
#include "RawAPI/VoxtaApiHandler.h"
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
	instanceData.m_ElapsedTime = 0.0f;
    instanceData.m_IsTryingToConnect = false;

	UVoxtaSubsystem* voxtaSubsystem = context.GetExternalDataPtr(m_VoxtaSubsystemHandle);
	ensure(voxtaSubsystem);

    if (!voxtaSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("[VoxtaConnectTask] Failed to resolve UVoxtaSubsystem."));
        return EStateTreeRunStatus::Failed;
    }

	const FString payload = FVoxtaApiHandler::BuildAuthenticatePayload(UVoxtaSubsystem::CLIENT_NAME, UVoxtaSubsystem::CLIENT_VERSION);
    instanceData.m_IsTryingToConnect = voxtaSubsystem->TrySend(payload);

    if (!instanceData.m_IsTryingToConnect)
    {
        UE_LOG(LogTemp, Warning, TEXT("[VoxtaConnectTask] Authenticate request failed to initiate."));
        return EStateTreeRunStatus::Failed;
    }

    return EStateTreeRunStatus::Running;
}
void FVoxtaConnectTask::ExitState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	FInstanceDataType& instanceData = context.GetInstanceData(*this);
    instanceData.m_IsTryingToConnect = false;
}
