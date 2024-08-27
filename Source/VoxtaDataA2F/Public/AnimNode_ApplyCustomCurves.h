// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "Animation/AnimNodeBase.h"
#include "CoreMinimal.h"
#include "VoxtaAudioPlaybackBase.h"
#include "AnimNode_ApplyCustomCurves.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct VOXTADATAA2F_API FAnimNode_ApplyCustomCurves : public FAnimNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
	FPoseLink Source;

	//~ begin FAnimNode_Base overrides
public:
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	virtual bool HasPreUpdate() const override { return true; }
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;
	//~ end FAnimNode_Base overrides

private:
	IVoxtaAudioPlaybackBase* CurveSource;

	TArray<float> CachedWeights;
};
