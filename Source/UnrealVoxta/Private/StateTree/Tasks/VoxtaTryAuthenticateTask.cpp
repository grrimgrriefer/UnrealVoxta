// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaTryAuthenticateTask.h"
#include "StateTreeExecutionContext.h"
#include "RawAPI/VoxtaApiHandler.h"
#include "StateTree/VoxtaStateTreeTags.h"

const UStruct* FVoxtaTryAuthenticateTask::GetInstanceDataType() const
{
    return FInstanceDataType::StaticStruct();
}
EStateTreeRunStatus FVoxtaTryAuthenticateTask::EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	if (Super::EnterState(context, transitions) == EStateTreeRunStatus::Failed)
	{
		return EStateTreeRunStatus::Failed;
	}

	FInstanceDataType& instanceData = context.GetInstanceData(*this);
	UVoxtaApiHandler* voxtaApiHandler = context.GetExternalDataPtr(m_VoxtaApiHandlerHandle);
	ensure(voxtaApiHandler);

    if (!voxtaApiHandler)
    {
        UE_LOG(LogTemp, Error, TEXT("[FVoxtaAuthenticateTask] Failed to resolve UVoxtaSubsystem."));
        return EStateTreeRunStatus::Failed;
    }

    bool sentRequest = voxtaApiHandler->TrySendAuthenticatePayload(UVoxtaApiHandler::CLIENT_NAME, UVoxtaApiHandler::CLIENT_VERSION);
    if (!sentRequest)
    {
        UE_LOG(LogTemp, Warning, TEXT("[FVoxtaAuthenticateTask] Authenticate request failed to initiate."));
        return EStateTreeRunStatus::Failed;
    }

	// TODO: Trigger TAG_Voxta_Mark_Authenticated once the server resonds with username and character list
	// GetSubsystem(context)->TrySendFlowEvent(TAG_Voxta_Mark_Authenticated, false, FConstStructView());

    return EStateTreeRunStatus::Running;
}
