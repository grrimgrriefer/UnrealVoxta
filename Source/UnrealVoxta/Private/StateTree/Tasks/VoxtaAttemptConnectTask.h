// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaBaseTask.h"
#include "VoxtaUserConfiguration.h"
#include "VoxtaAttemptConnectTask.generated.h"

class UVoxtaApiHandler;

USTRUCT(BlueprintType)
struct FVoxtaConnectPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "UserConfig")
	FVoxtaUserConfiguration m_UserConfig;
};

USTRUCT()
struct UNREALVOXTA_API FVoxtaConnectTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	FVoxtaUserConfiguration m_UserConfig;
};

/**
 * Handles the lifetime of the connection the VoxtaServer backend.
 */
USTRUCT(meta = (DisplayName = "Voxta Connect", Category = "Voxta"))
struct UNREALVOXTA_API FVoxtaAttemptConnectTask : public FVoxtaBaseTask
{
	GENERATED_BODY()

public:
	using FInstanceDataType = FVoxtaConnectTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const override;

protected:
	virtual VoxtaClientState GetStateForTask(FStateTreeExecutionContext& context) const override { return VoxtaClientState::AttemptingToConnect; }

private:
	TStateTreeExternalDataHandle<UVoxtaApiHandler> m_VoxtaApiHandlerHandle;
};
