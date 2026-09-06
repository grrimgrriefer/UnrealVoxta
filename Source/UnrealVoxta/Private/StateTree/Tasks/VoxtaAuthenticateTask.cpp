// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAuthenticateTask.h"
#include "StateTreeExecutionContext.h"
#include "RawAPI/VoxtaApiHandler.h"

FVoxtaAuthenticateTask::FVoxtaAuthenticateTask()
{
	bShouldCallTick = false;
}
const UStruct* FVoxtaAuthenticateTask::GetInstanceDataType() const
{
    return FInstanceDataType::StaticStruct();
}
EStateTreeRunStatus FVoxtaAuthenticateTask::EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	FInstanceDataType& instanceData = context.GetInstanceData(*this);
	UVoxtaApiHandler* voxtaSocketHandler = context.GetExternalDataPtr(m_VoxtaApiHandlerHandle);
	ensure(voxtaSocketHandler);

    if (!voxtaSocketHandler)
    {
        UE_LOG(LogTemp, Error, TEXT("[FVoxtaAuthenticateTask] Failed to resolve UVoxtaSubsystem."));
        return EStateTreeRunStatus::Failed;
    }

    bool sentRequest = voxtaSocketHandler->TrySendAuthenticatePayload(UVoxtaApiHandler::CLIENT_NAME, UVoxtaApiHandler::CLIENT_VERSION);
    if (!sentRequest)
    {
        UE_LOG(LogTemp, Warning, TEXT("[FVoxtaAuthenticateTask] Authenticate request failed to initiate."));
        return EStateTreeRunStatus::Failed;
    }

    return EStateTreeRunStatus::Running;
}
