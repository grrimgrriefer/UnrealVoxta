// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaTryAuthenticateTask.h"
#include "StateTreeExecutionContext.h"
#include "VoxtaGlobals.h"
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
	ensureAlways(voxtaApiHandler);

    if (!voxtaApiHandler)
    {
		UE_LOGFMT(LogVoxta, Error, "[{0}] Failed to resolve UVoxtaApiHandler.", StaticStruct()->GetName());
        return EStateTreeRunStatus::Failed;
    }

    bool sentRequest = voxtaApiHandler->TrySendAuthenticatePayload(UVoxtaApiHandler::CLIENT_NAME, UVoxtaApiHandler::CLIENT_VERSION);
    if (!sentRequest)
    {
        UE_LOGFMT(LogVoxta, Warning, "[{0}] Authenticate request failed to initiate.", StaticStruct()->GetName());
        return EStateTreeRunStatus::Failed;
    }

	// TODO: Trigger TAG_Voxta_Mark_Authenticated once the server resonds with username and character list
	// GetSubsystem(context)->TrySendFlowEvent(TAG_Voxta_Mark_Authenticated, false, FConstStructView());

    return EStateTreeRunStatus::Running;
}
