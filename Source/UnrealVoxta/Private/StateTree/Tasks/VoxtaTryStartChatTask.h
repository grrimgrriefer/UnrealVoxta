// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaBaseTask.h"
#include "VoxtaClientState.h"
#include "VoxtaTryStartChatTask.generated.h"

class UVoxtaApiHandler;

USTRUCT(BlueprintType)
struct UNREALVOXTA_API FVoxtaTryStartChatPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChatConfig")
	FString m_ChatCharacter = TEXT("");
};

USTRUCT()
struct UNREALVOXTA_API FVoxtaTryStartChatTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	FString m_ChatCharacter = TEXT("");
};

/**
 * Triggers an attempt to start a chat on the VoxtaServer
 */
USTRUCT(meta = (DisplayName = "Voxta Attempt to start a chat", Category = "Voxta"))
struct UNREALVOXTA_API FVoxtaTryStartChatTask : public FVoxtaBaseTask
{
	GENERATED_BODY()

public:
	using FInstanceDataType = FVoxtaTryStartChatTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const override;

protected:
	virtual VoxtaClientState GetStateForTask(FStateTreeExecutionContext& context) const override { return VoxtaClientState::AttemptingToStartChat; }
};
