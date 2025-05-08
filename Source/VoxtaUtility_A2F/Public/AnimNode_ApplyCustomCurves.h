// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_ApplyCustomCurves.generated.h"

class IA2FWeightProvider;

/**
 * FAnimNode_ApplyCustomCurves
 * Animation node that applies custom curve values to a pose.
 * Used for driving facial animation curves (e.g., from lipsync data) in an animation graph.
 */
USTRUCT(BlueprintInternalUseOnly, Category = "Voxta")
struct VOXTAUTILITY_A2F_API FAnimNode_ApplyCustomCurves : public FAnimNode_Base
{
	GENERATED_BODY()

#pragma region FAnimNode_Base overrides
public:
	/**
	 * Update the state of the graph relative to this node.
	 * Configures any weights or state that could affect the poses to be evaluated.
	 * Can be called on any thread.
	 *
	 * @param Context Context structure providing access to relevant data.
	 */
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;

	/**
	 * Evaluate local-space bone transforms according to the weights set up in Update().
	 * You should implement either Evaluate or EvaluateComponentSpace, but not both.
	 * Can be called on any thread.
	 *
	 * @param Output Output structure to write pose or curve data to.
	 */
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;

	/**
	 * Gather on-screen debug data.
	 * Called on the game thread.
	 *
	 * @param DebugData Debug data structure used to output any relevant data.
	 */
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;

	/**
	 * Indicates that PreUpdate() should be called on the game thread before Update().
	 * Used to gather non-thread safe data.
	 * Called on the game thread.
	 */
	virtual bool HasPreUpdate() const override { return true; }

	/**
	 * Perform game-thread work prior to non-game thread Update() being called.
	 * Called on the game thread.
	 *
	 * @param AnimInstance The animation instance.
	 */
	virtual void PreUpdate(const UAnimInstance* AnimInstance) override;
#pragma endregion

#pragma region data
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Source"))
	FPoseLink m_source;

	IA2FWeightProvider* m_curveSource = nullptr;
	TArray<float> m_cachedWeights = TArray<float>();
#pragma endregion
};
