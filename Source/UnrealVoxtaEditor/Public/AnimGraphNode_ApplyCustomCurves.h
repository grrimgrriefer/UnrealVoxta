// Copyright 2018 Sean Chen. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "AnimNode_ApplyCustomCurves.h"
#include "AnimGraphNode_ApplyCustomCurves.generated.h"

/**
 * The node Editor UI for the FAnimNode_ApplyCustomCurves node where we apply the custom A2F curves
 * to the ARKit mapping.
 *
 * TODO: Maybe rename Editor module, depends if this turns out to be the only thing or not.
 */
UCLASS(Category = "Voxta")
class UNREALVOXTAEDITOR_API UAnimGraphNode_ApplyCustomCurves : public UAnimGraphNode_Base
{
	GENERATED_BODY()

#pragma region data
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Node"))
	FAnimNode_ApplyCustomCurves m_node;
#pragma endregion
};