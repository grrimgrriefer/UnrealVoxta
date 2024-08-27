// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaAudioPlaybackBase.generated.h"

/// <summary>
/// Base struct so it can be used without a hard req on the UnrealVoxta module
/// </summary>
UINTERFACE(MinimalAPI)
class UVoxtaAudioPlaybackBase : public UInterface
{
	GENERATED_BODY()
};

class VOXTADATA_API IVoxtaAudioPlaybackBase
{
	GENERATED_BODY()

public:
	virtual void GetA2FCurveWeights(TArray<float>& targetArrayRef)
	{
	}
};
