// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AnimNode_ApplyCustomCurves.h"
#include "Animation/AnimNodeBase.h"
#include "Audio2FacePlaybackHandler.h"
#include "AbstractA2FWeightProvider.h"
#include "Logging/StructuredLog.h"
#include "VoxtaDefines.h"
#include "Components/AudioComponent.h"

void FAnimNode_ApplyCustomCurves::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);
	m_source.Update(Context);
}

void FAnimNode_ApplyCustomCurves::PreUpdate(const UAnimInstance* AnimInstance)
{
	if (!(AnimInstance->GetWorld()->WorldType == EWorldType::PIE) &&
		!(AnimInstance->GetWorld()->WorldType == EWorldType::Game))
	{
		// Check if we're playing, otherwise this will trigger in blueprint editor when previewing
		return;
	}
	m_cachedWeights.Init(0.f, UE_ARRAY_COUNT(UAudio2FacePlaybackHandler::CURVE_NAMES));

	if (m_curveSource == nullptr)
	{
		AActor* actor = AnimInstance->GetOwningActor();
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

void FAnimNode_ApplyCustomCurves::Evaluate_AnyThread(FPoseContext& Output)
{
	m_source.Evaluate(Output);

	if (m_cachedWeights.Num() > 0)
	{
		if (m_cachedWeights.Num() != UAudio2FacePlaybackHandler::CURVE_COUNT)
		{
			UE_LOGFMT(VoxtaLog, Error, "A2F curve count was different then ARKIT curve count, this is not supported.");
			return;
		}

		for (int i = 0; i < m_cachedWeights.Num(); ++i)
		{
			Output.Curve.Set(UAudio2FacePlaybackHandler::CURVE_NAMES[i], m_cachedWeights[i]);
		}
	}
}

void FAnimNode_ApplyCustomCurves::GatherDebugData(FNodeDebugData& DebugData)
{
	FAnimNode_Base::GatherDebugData(DebugData);
	m_source.GatherDebugData(DebugData);
}