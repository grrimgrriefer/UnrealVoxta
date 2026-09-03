// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "VoxtaConnectTask.generated.h"

class UVoxtaSocketHandler;
class UVoxtaStateTreeSubsystem;

USTRUCT(BlueprintType)
struct UNREALVOXTA_API FVoxtaConnectTaskInstanceData
{
	GENERATED_BODY()
};

/**
 * Handles the lifetime of the connection the VoxtaServer backend.
 */
USTRUCT(meta = (DisplayName = "Voxta Connect", Category = "Voxta"))
struct UNREALVOXTA_API FVoxtaConnectTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

public:
	using FInstanceDataType = FVoxtaConnectTaskInstanceData;

	FVoxtaConnectTask();

	virtual const UStruct* GetInstanceDataType() const override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const override;
	virtual void ExitState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const override;

	TStateTreeExternalDataHandle<UVoxtaStateTreeSubsystem> m_VoxtaSubSystemHandle;
	TStateTreeExternalDataHandle<UVoxtaSocketHandler> m_VoxtaSocketHandlerHandle;
};
