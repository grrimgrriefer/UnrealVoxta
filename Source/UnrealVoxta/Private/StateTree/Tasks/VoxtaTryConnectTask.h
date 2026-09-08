// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaBaseTask.h"
#include "VoxtaClientState.h"
#include "VoxtaTryConnectTask.generated.h"

class UVoxtaApiHandler;

USTRUCT(BlueprintType)
struct UNREALVOXTA_API FVoxtaConnectPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UserConfig")
	FString m_VoxtaServerIpv4 = TEXT("127.0.0.1");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UserConfig")
	int32 m_VoxtaServerPort = 5384;
};

USTRUCT()
struct UNREALVOXTA_API FVoxtaConnectTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	FString m_VoxtaServerIpv4 = TEXT("127.0.0.1");
	UPROPERTY(EditAnywhere, Category = "Input")
	int32 m_VoxtaServerPort = 5384;
};

/**
 * Triggers an attempt to connect to the VoxtaServer
 */
USTRUCT(meta = (DisplayName = "Voxta Attempt to connect", Category = "Voxta"))
struct UNREALVOXTA_API FVoxtaTryConnectTask : public FVoxtaBaseTask
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
