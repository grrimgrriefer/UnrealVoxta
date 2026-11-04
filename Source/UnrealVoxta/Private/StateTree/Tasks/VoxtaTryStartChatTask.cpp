// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaTryStartChatTask.h"
#include "StateTreeExecutionContext.h"
#include "VoxtaGlobals.h"
#include "RawAPI/VoxtaApiHandler.h"
#include "RawAPI/VoxtaPayloads.h"
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

	instanceData.m_ChatStartingDelegateHandle = voxtaApiHandler->OnResponse<FVoxtaPayload_ChatStartingResponse>().AddLambda(
		[weakContext = context.MakeWeakExecutionContext()](const FVoxtaPayload_ChatStartingResponse& response)
		{
			const FStateTreeStrongExecutionContext strongContext = weakContext.MakeStrongExecutionContext();
			if (!strongContext.IsValid())
			{
				return;
			}

			if (UVoxtaStateTreeSubsystem* subsystem = Cast<UVoxtaStateTreeSubsystem>(strongContext.GetOwner().Get()))
			{
				// TODO: set chat session id in subsystem (response.sessionId)
				// subsystem->
			}
		});

	instanceData.m_ChatLoadingDelegateHandle = voxtaApiHandler->OnResponse<FVoxtaPayload_ChatLoadingMessageResponse>().AddLambda(
		[weakContext = context.MakeWeakExecutionContext()](const FVoxtaPayload_ChatLoadingMessageResponse& response)
		{
			const FStateTreeStrongExecutionContext strongContext = weakContext.MakeStrongExecutionContext();
			if (!strongContext.IsValid())
			{
				return;
			}

			if (UVoxtaStateTreeSubsystem* subsystem = Cast<UVoxtaStateTreeSubsystem>(strongContext.GetOwner().Get()))
			{
				subsystem->NotifyChatLoadingProgress(response.sessionId, FText::FromString(response.text), response.progress);
			}
		});

	instanceData.m_ChatStartedDelegateHandle = voxtaApiHandler->OnResponse<FVoxtaPayload_ChatStartedResponse>().AddLambda(
		[weakContext = context.MakeWeakExecutionContext()](const FVoxtaPayload_ChatStartedResponse& response)
		{
			const FStateTreeStrongExecutionContext strongContext = weakContext.MakeStrongExecutionContext();
			if (!strongContext.IsValid())
			{
				return;
			}

			if (UVoxtaStateTreeSubsystem* subsystem = Cast<UVoxtaStateTreeSubsystem>(strongContext.GetOwner().Get()))
			{
				subsystem->NotifyChatStarted(response.sessionId, response.chatId);
			}
		});

	FString debugContext = TEXT("Just a debug chat session, in an empty void. Kinda cold here tbh.");
	voxtaApiHandler->TrySendStartChatPayload(instanceData.m_ChatCharacter, debugContext);

	return EStateTreeRunStatus::Running;
}
void FVoxtaTryStartChatTask::ExitState(
	FStateTreeExecutionContext& context,
	const FStateTreeTransitionResult& transitions) const
{
	FInstanceDataType& instanceData = context.GetInstanceData(*this);
	UVoxtaApiHandler* voxtaApiHandler = context.GetExternalDataPtr(m_VoxtaApiHandlerHandle);

	if (voxtaApiHandler)
	{
		if (instanceData.m_ChatStartingDelegateHandle.IsValid())
		{
			voxtaApiHandler->OnResponse<FVoxtaPayload_ChatStartingResponse>().Remove(
				instanceData.m_ChatStartingDelegateHandle
			);
			instanceData.m_ChatStartingDelegateHandle.Reset();
		}
		if (instanceData.m_ChatLoadingDelegateHandle.IsValid())
		{
			voxtaApiHandler->OnResponse<FVoxtaPayload_ChatLoadingMessageResponse>().Remove(
				instanceData.m_ChatLoadingDelegateHandle
			);
			instanceData.m_ChatLoadingDelegateHandle.Reset();
		}
		if (instanceData.m_ChatStartedDelegateHandle.IsValid())
		{
			voxtaApiHandler->OnResponse<FVoxtaPayload_ChatStartedResponse>().Remove(
				instanceData.m_ChatStartedDelegateHandle
			);
			instanceData.m_ChatStartedDelegateHandle.Reset();
		}
	}

	Super::ExitState(context, transitions);
}
