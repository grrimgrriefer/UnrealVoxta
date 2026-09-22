// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaBaseTask.h"
#include "VoxtaClientState.h"
#include "VoxtaTryAuthenticateTask.generated.h"

class UVoxtaApiHandler;

USTRUCT(BlueprintType)
struct UNREALVOXTA_API FVoxtaAuthenticateTaskInstanceData
{
	GENERATED_BODY()

	FDelegateHandle m_WelcomeDelegateHandle;
	FDelegateHandle m_ConfigurationDelegateHandle;
	FDelegateHandle m_CharacterListDelegateHandle;
};

/**
 * Triggers an attempt to authenticate to the VoxtaServer
 * Also fetches the core info (user, characters, services-config)
 */
USTRUCT(meta = (DisplayName = "Voxta Attempt to authenticate", Category = "Voxta"))
struct UNREALVOXTA_API FVoxtaTryAuthenticateTask : public FVoxtaBaseTask
{
	GENERATED_BODY()

public:
	using FInstanceDataType = FVoxtaAuthenticateTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const override;
	virtual void ExitState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const override;

protected:
	virtual VoxtaClientState GetStateForTask(FStateTreeExecutionContext& context) const override { return VoxtaClientState::AttemptingToAuthenticate; }
};
