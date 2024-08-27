// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AnimNode_ApplyCustomCurves.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNodeBase.h"
#include "Audio2FacePlaybackHandler.h"

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
			UAudioComponent* FoundComponent = Actor->GetComponentByClass<UAudioComponent>();
			if (FoundComponent)
			{
				CurveSource = Cast<IVoxtaAudioPlaybackBase>(FoundComponent);
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
			FName CurveName = UAudio2FacePlaybackHandler::CurveNames[CurveIdx++];
			Output.Curve.Set(CurveName, Weight);
		}
	}
}

void FAnimNode_ApplyCustomCurves::GatherDebugData(FNodeDebugData& DebugData)
{
	FAnimNode_Base::GatherDebugData(DebugData);
	Source.GatherDebugData(DebugData);
}