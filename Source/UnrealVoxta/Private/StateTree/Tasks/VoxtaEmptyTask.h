// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaBaseTask.h"
#include "VoxtaClientState.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "VoxtaEmptyTask.generated.h"

USTRUCT()
struct FVoxtaEmptyTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxta Client State")
	VoxtaClientState m_ClientStateForThisTask = VoxtaClientState::Invalid;
};

/**
 * Empty task to keep track of states that don't actively 'do' anything
 */
USTRUCT(meta = (DisplayName = "Voxta Empty Task", Category = "Voxta"))
struct UNREALVOXTA_API FVoxtaEmptyTask : public FVoxtaBaseTask
{
	GENERATED_BODY()

public:
	using FInstanceDataType =FVoxtaEmptyTaskInstanceData;

	virtual const UScriptStruct* GetInstanceDataType() const override;

protected:
	virtual VoxtaClientState GetStateForTask(FStateTreeExecutionContext& context) const override;
};
