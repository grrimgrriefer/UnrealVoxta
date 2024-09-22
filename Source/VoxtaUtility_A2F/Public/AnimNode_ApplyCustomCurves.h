// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_ApplyCustomCurves.generated.h"

class IA2FWeightProvider;

/**
 * FAnimNode_ApplyCustomCurves.
 * Custom animation node to apply predifined curves (float values per frame) to the ARKit mapping of the rig.
 */
USTRUCT(BlueprintInternalUseOnly, Category = "Voxta")
struct VOXTAUTILITY_A2F_API FAnimNode_ApplyCustomCurves : public FAnimNode_Base
{
	GENERATED_BODY()

#pragma region FAnimNode_Base overrides
public:
	/**
	 * Called to update the state of the graph relative to this node.
	 * Generally this should configure any weights (etc.) that could affect the poses that
	 * will need to be evaluated. This function is what usually executes EvaluateGraphExposedInputs.
	 * This can be called on any thread.
	 * @param	Context		Context structure providing access to relevant data
	 */
	virtual void Update_AnyThread(const FAnimationUpdateContext& context) override;

	/**
	 * Called to evaluate local-space bones transforms according to the weights set up in Update().
	 * You should implement either Evaluate or EvaluateComponentSpace, but not both of these.
	 * This can be called on any thread.
	 * @param	Output		Output structure to write pose or curve data to. Also provides access to relevant data as a context.
	 */
	virtual void Evaluate_AnyThread(FPoseContext& output) override;

	/**
	 * Called to gather on-screen debug data.
	 * This is called on the game thread.
	 * @param	DebugData	Debug data structure used to output any relevant data
	 */
	virtual void GatherDebugData(FNodeDebugData& debugData) override;

	/**
	 * Override this to indicate that PreUpdate() should be called on the game thread (usually to
	 * gather non-thread safe data) before Update() is called.
	 * Note that this is called at load on the UAnimInstance CDO to avoid needing to call this at runtime.
	 * This is called on the game thread.
	 */
	virtual bool HasPreUpdate() const override { return true; }

	/** Override this to perform game-thread work prior to non-game thread Update() being called */
	virtual void PreUpdate(const UAnimInstance* animInstance) override;
#pragma endregion

#pragma region data
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Source"))
	FPoseLink m_source;

	IA2FWeightProvider* m_curveSource;
	TArray<float> m_cachedWeights;
#pragma endregion
};
