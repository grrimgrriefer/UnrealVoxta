// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "EmptyTask.generated.h"

USTRUCT()
struct FEmptyTaskInstanceData
{
	GENERATED_BODY()
};

/**
 * Empty task to prevent the StateTree from transitioning.
 */
USTRUCT(meta = (DisplayName = "Empty Task", Category = "Voxta"))
struct UNREALVOXTA_API FEmptyTask : public FStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEmptyTaskInstanceData;

	FEmptyTask();

	virtual const UScriptStruct* GetInstanceDataType() const override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const override;
};
