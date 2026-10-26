// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaTryAuthenticateTask.h"
#include "StateTreeAsyncExecutionContext.h"
#include "StateTreeExecutionContext.h"
#include "VoxtaGlobals.h"
#include "VoxtaRuntimeData.h"
#include "RawAPI/VoxtaApiHandler.h"
#include "RawAPI/VoxtaPayloads.h"
#include "StateTree/VoxtaStateTreeTags.h"
#include "SubSystems/VoxtaStateTreeSubsystem.h"

struct FResponseContainer
{
	TOptional<FVoxtaPayload_WelcomeResponse> m_welcomeResponse;
	TOptional<FVoxtaPayload_ConfigurationResponse> m_configResponse;
	TOptional<FVoxtaPayload_CharacterListLoadedResponse> m_characterListResponse;

	bool IsComplete() const
	{
		return m_welcomeResponse.IsSet() &&
			m_configResponse.IsSet() &&
			m_characterListResponse.IsSet();
	}
};

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
		return EStateTreeRunStatus::Failed;
	}

	TSharedRef<FResponseContainer> responseContainer = MakeShared<FResponseContainer>();

	auto CheckAndFinish = [weakContext = context.MakeWeakExecutionContext(), responseContainer]()
	{
		if (!responseContainer->IsComplete())
		{
			return;
		}

		const FStateTreeStrongExecutionContext strongContext = weakContext.MakeStrongExecutionContext();
		if (!strongContext.IsValid())
		{
			return;
		}

		if (UVoxtaStateTreeSubsystem* subsystem = Cast<UVoxtaStateTreeSubsystem>(strongContext.GetOwner().Get()))
		{
			FVoxtaRuntimeData runtimeData;
			runtimeData.m_User.m_Id = responseContainer->m_welcomeResponse->user.id;
			runtimeData.m_User.m_Name = responseContainer->m_welcomeResponse->user.name;
			runtimeData.m_ServerVersion = responseContainer->m_welcomeResponse->voxtaServerVersion;

			for (const FVoxtaPayload_CharacterData& responsePayloadChar : responseContainer->m_characterListResponse->characters)
			{
				FVoxtaCharacterData charData;
				charData.m_Id = responsePayloadChar.id;
				charData.m_Name = responsePayloadChar.name;
				charData.m_CreatorNotes = responsePayloadChar.creatorNotes;
				charData.m_IsAllowExplicitContent = responsePayloadChar.explicitContent;
				charData.m_IsFavorite = responsePayloadChar.favorite;
				charData.m_ThumbnailUrl = responsePayloadChar.thumbnailUrl;
				runtimeData.m_Characters.Add(MoveTemp(charData));
			}

			// TODO: Figure out if we actually need these or not
//			runtimeData.m_Assistant.m_Id = responseContainer->m_welcomeResponse->assistant.id;
//			runtimeData.m_Assistant.m_Name = responseContainer->m_welcomeResponse->assistant.name;
//			runtimeData.m_Services.m_DefaultTextGenId = responseContainer->m_configResponse->configurations[0].services.textGen.defaultServiceId;
//			runtimeData.m_Services.m_DefaultSpeechToTextId = responseContainer->m_configResponse->configurations[0].services.speechToText.defaultServiceId;
//			runtimeData.m_Services.m_DefaultTextToSpeechId = responseContainer->m_configResponse->configurations[0].services.textToSpeech.defaultServiceId;

			subsystem->SetRuntimeData(runtimeData);
			subsystem->TrySendFlowEvent(TAG_Voxta_Signal_Authenticated, false, FConstStructView());
		}
	};

	instanceData.m_WelcomeDelegateHandle = voxtaApiHandler->OnResponse<FVoxtaPayload_WelcomeResponse>().AddLambda(
		[responseContainer, CheckAndFinish, voxtaApiHandler](const FVoxtaPayload_WelcomeResponse& response)
		{
			if (response.apiVersion != UVoxtaApiHandler::COMPATIBLE_SERVER_VERSION)
			{
				UE_LOGFMT(LogVoxta, Error, "[{0}] Incompatible server version. Expecting [{1}] but received [{2}]",
					StaticStruct()->GetName(),
					UVoxtaApiHandler::COMPATIBLE_SERVER_VERSION,
					response.apiVersion);

				// TODO: what do we do here? Need to trigger event for UI prolly
			}

			UE_LOGFMT(LogVoxta, Log, "[{0}] Received Welcome response for user: {1}", StaticStruct()->GetName(), response.user.name);
			responseContainer->m_welcomeResponse = response;

			voxtaApiHandler->TrySendLoadCharactersPayload();
			CheckAndFinish();
		});

	instanceData.m_ConfigurationDelegateHandle = voxtaApiHandler->OnResponse<FVoxtaPayload_ConfigurationResponse>().AddLambda(
		[responseContainer, CheckAndFinish](const FVoxtaPayload_ConfigurationResponse& response)
		{
			UE_LOGFMT(LogVoxta, Log, "[{0}] Received Configuration response.", StaticStruct()->GetName());
			responseContainer->m_configResponse = response;
			CheckAndFinish();
		});

	instanceData.m_CharacterListDelegateHandle = voxtaApiHandler->OnResponse<FVoxtaPayload_CharacterListLoadedResponse>().AddLambda(
		[responseContainer, CheckAndFinish](const FVoxtaPayload_CharacterListLoadedResponse& response)
		{
			UE_LOGFMT(LogVoxta, Log, "[{0}] Received CharacterList response.", StaticStruct()->GetName());
			responseContainer->m_characterListResponse = response;
			CheckAndFinish();
		});

	const bool sentRequest = voxtaApiHandler->TrySendAuthenticatePayload(UVoxtaApiHandler::CLIENT_NAME, UVoxtaApiHandler::CLIENT_VERSION);
	if (!sentRequest)
	{
		UE_LOGFMT(LogVoxta, Error, "[{0}] Authenticate request failed to initiate.", StaticStruct()->GetName());
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

void FVoxtaTryAuthenticateTask::ExitState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const
{
	FInstanceDataType& instanceData = context.GetInstanceData(*this);
	UVoxtaApiHandler* voxtaApiHandler = context.GetExternalDataPtr(m_VoxtaApiHandlerHandle);

	if (voxtaApiHandler)
	{
		if (instanceData.m_WelcomeDelegateHandle.IsValid())
		{
			voxtaApiHandler->OnResponse<FVoxtaPayload_WelcomeResponse>().Remove(instanceData.m_WelcomeDelegateHandle);
			instanceData.m_WelcomeDelegateHandle.Reset();
		}
		if (instanceData.m_ConfigurationDelegateHandle.IsValid())
		{
			voxtaApiHandler->OnResponse<FVoxtaPayload_ConfigurationResponse>().Remove(instanceData.m_ConfigurationDelegateHandle);
			instanceData.m_ConfigurationDelegateHandle.Reset();
		}
		if (instanceData.m_CharacterListDelegateHandle.IsValid())
		{
			voxtaApiHandler->OnResponse<FVoxtaPayload_CharacterListLoadedResponse>().Remove(instanceData.m_CharacterListDelegateHandle);
			instanceData.m_CharacterListDelegateHandle.Reset();
		}
	}

	Super::ExitState(context, transitions);
}
