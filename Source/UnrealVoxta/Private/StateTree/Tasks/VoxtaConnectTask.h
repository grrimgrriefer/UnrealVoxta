// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "VoxtaConnectTask.generated.h"

class UVoxtaSubsystem;

USTRUCT(BlueprintType)
struct UNREALVOXTA_API FVoxtaConnectTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float m_TimeoutSeconds = 15.0f;

	float m_ElapsedTime = 0.0f;
	bool m_IsTryingToConnect = false;
};

/**
 * Handles connecting and authentication handshake with the VoxtaServer backend.
 */
USTRUCT(meta = (DisplayName = "Voxta Connect & Authenticate", Category = "Voxta"))
struct UNREALVOXTA_API FVoxtaConnectTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

public:
	using FInstanceDataType = FVoxtaConnectTaskInstanceData;

	FVoxtaConnectTask();

	virtual const UStruct* GetInstanceDataType() const override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const override;
	virtual void ExitState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const override;

	TStateTreeExternalDataHandle<UVoxtaSubsystem> m_VoxtaSubsystemHandle;
};
