// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaTryStartChatTask.h"
#include "StateTreeExecutionContext.h"
#include "VoxtaGlobals.h"
#include "RawAPI/VoxtaApiHandler.h"
#include "SubSystems/VoxtaStateTreeSubsystem.h"

const UStruct* FVoxtaTryStartChatTask::GetInstanceDataType() const
{
	return FInstanceDataType::StaticStruct();
}
EStateTreeRunStatus FVoxtaTryStartChatTask::EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
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

	FString debugContext = TEXT("Just a debug chat session, in an empty void. Kinda cold here tbh.");
	voxtaApiHandler->TrySendStartChatPayload(instanceData.m_ChatCharacter, debugContext);

	return EStateTreeRunStatus::Running;
}
