// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AnimNode_ApplyCustomCurves.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNodeBase.h"

void FAnimNode_ApplyCustomCurves::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);
	Source.Update(Context);
}

void FAnimNode_ApplyCustomCurves::PreUpdate(const UAnimInstance* InAnimInstance)
{
	if (CurveSource == nullptr)
	{
		AActor* Actor = InAnimInstance->GetOwningActor();
		if (Actor != nullptr)
		{
			UVoxtaAudioPlayback* voxtaComponent = Actor->FindComponentByClass<UVoxtaAudioPlayback>();
			if (voxtaComponent != nullptr)
			{
				CurveSource = voxtaComponent;
			}
		}
	}

	if (CurveSource != nullptr)
	{
		CachedWeights.Reset();
		CurveSource->GetA2FCurveWeights(CachedWeights);
	}
}

void FAnimNode_ApplyCustomCurves::Evaluate_AnyThread(FPoseContext& Output)
{
	Source.Evaluate(Output);

	if (CachedWeights.Num() > 0)
	{
		size_t CurveIdx = 0;
		for (float Weight : CachedWeights)
		{
			FName CurveName = UACEAudioCurveSourceComponent::CurveNames[CurveIdx++];
			Output.Curve.Set(CurveName, Weight);
			TRACE_ANIM_NODE_VALUE(Output, *CurveName.ToString(), Weight);
		}
	}
}

void FAnimNode_ApplyCustomCurves::GatherDebugData(FNodeDebugData& DebugData)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(GatherDebugData)
	FAnimNode_Base::GatherDebugData(DebugData);
	Source.GatherDebugData(DebugData);
}