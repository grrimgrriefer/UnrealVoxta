// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaBaseTask.h"
#include "VoxtaAuthenticateTask.generated.h"

class UVoxtaApiHandler;

USTRUCT(BlueprintType)
struct UNREALVOXTA_API FVoxtaAuthenticateTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float m_TimeoutSeconds = 15.0f;
};

/**
 * Handles the lifetime of the authenticated session with the VoxtaServer backend.
 */
USTRUCT(meta = (DisplayName = "Voxta Authenticate", Category = "Voxta"))
struct UNREALVOXTA_API FVoxtaAuthenticateTask : public FVoxtaBaseTask
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
