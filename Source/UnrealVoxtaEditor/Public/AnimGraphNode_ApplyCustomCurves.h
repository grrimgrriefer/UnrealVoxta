// Copyright 2018 Sean Chen. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimNode_ApplyCustomCurves.h"
#include "AnimGraphNode_Base.h"
#include "AnimGraphNode_ApplyCustomCurves.generated.h"

struct FAnimNode_ApplyCustomCurves;

UCLASS()
class UNREALVOXTAEDITOR_API UAnimGraphNode_ApplyCustomCurves : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Settings")
	FAnimNode_ApplyCustomCurves Node;
};