// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AnimNode_ApplyCustomCurves.h"
#include "Animation/AnimNodeBase.h"
#include "Audio2FacePlaybackHandler.h"
#include "AbstractA2FWeightProvider.h"
#include "Logging/StructuredLog.h"
#include "VoxtaDefines.h"

void FAnimNode_ApplyCustomCurves::Update_AnyThread(const FAnimationUpdateContext& context)
{
	GetEvaluateGraphExposedInputs().Execute(context);
	m_source.Update(context);
}

void FAnimNode_ApplyCustomCurves::PreUpdate(const UAnimInstance* animInstance)
{
	if (!(animInstance->GetWorld()->WorldType == EWorldType::PIE) &&
		!(animInstance->GetWorld()->WorldType == EWorldType::Game))
	{
		// Check if we're playing, otherwise this will trigger in blueprint editor when previewing
		return;
	}

	if (m_curveSource == nullptr)
	{
		AActor* actor = animInstance->GetOwningActor();
		if (actor != nullptr)
		{
			UAudioComponent* component = actor->GetComponentByClass<UAudioComponent>();
			if (component)
			{
				m_curveSource = Cast<IA2FWeightProvider>(component);
			}
			else
			{
				UE_LOGFMT(VoxtaLog, Error, "Could not find the A2F curveweight provider on the character. "
					"Did you forget to add the VoxtaAudioPlayback component?");
			}
		}
		else
		{
			UE_LOGFMT(VoxtaLog, Error, "A2F curves can only be applied to actors & their derivatives. "
				"Other UObject are not supported.");
		}
	}

	if (m_curveSource != nullptr)
	{
		m_cachedWeights.Reset();
		m_curveSource->GetA2FCurveWeightsPreUpdate(m_cachedWeights);
	}
}

void FAnimNode_ApplyCustomCurves::Evaluate_AnyThread(FPoseContext& output)
{
	m_source.Evaluate(output);

	if (m_cachedWeights.Num() > 0)
	{
		size_t curveId = 0;
		for (float weight : m_cachedWeights)
		{
			FName curveName = UAudio2FacePlaybackHandler::CURVE_NAMES[curveId++];
			output.Curve.Set(curveName, weight);
		}
	}
}

void FAnimNode_ApplyCustomCurves::GatherDebugData(FNodeDebugData& debugData)
{
	FAnimNode_Base::GatherDebugData(debugData);
	m_source.GatherDebugData(debugData);
}