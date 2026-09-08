// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "SubSystems/VoxtaStateTreeSubsystem.h"
#include "VoxtaBaseTask.generated.h"

/**
 * Base task that registers itself as active/inactive with the subsystem.
 * So external entities can query which state we are in.
 */
USTRUCT(meta = (DisplayName = "Base Task", Category = "Voxta"))
struct UNREALVOXTA_API FVoxtaBaseTask : public FStateTreeTaskBase
{
	GENERATED_BODY()

	FVoxtaBaseTask();

	virtual bool Link(FStateTreeLinker& linker) override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const override;
	virtual void ExitState(FStateTreeExecutionContext& context, const FStateTreeTransitionResult& transitions) const override;

protected:
	virtual VoxtaClientState GetStateForTask(FStateTreeExecutionContext& context) const;

	TStateTreeExternalDataHandle<UVoxtaStateTreeSubsystem> m_VoxtaStateTreeSubsystemHandle;
};
