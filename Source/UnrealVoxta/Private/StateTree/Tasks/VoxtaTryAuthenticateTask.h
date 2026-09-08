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
};

/**
 * Triggers an attempt to connect to the VoxtaServer
 */
USTRUCT(meta = (DisplayName = "Voxta Attempt to authenticate", Category = "Voxta"))
struct UNREALVOXTA_API FVoxtaTryAuthenticateTask : public FVoxtaBaseTask
{
	GENERATED_BODY()

public:
	using FInstanceDataType = FVoxtaAuthenticateTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const override;

protected:
	virtual VoxtaClientState GetStateForTask(FStateTreeExecutionContext& context) const override { return VoxtaClientState::AttemptingToAuthenticate; }

private:
	TStateTreeExternalDataHandle<UVoxtaApiHandler> m_VoxtaApiHandlerHandle;
};
