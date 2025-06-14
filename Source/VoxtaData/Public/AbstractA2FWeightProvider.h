// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "AbstractA2FWeightProvider.generated.h"

/**
 * UA2FWeightProvider
 * Used by UE to ensure the IA2FWeightProvider interface is picked up by Unreal's Reflection system
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UA2FWeightProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * IA2FWeightProvider
 * The interface implemented by whoever can provide the A2F curves for any given update / frame.
 * In practice this is mainly used to avoid modules being dependent on UnrealVoxta, as that should be the highest level,
 * and nothing should dependent on it.
 */
class VOXTADATA_API IA2FWeightProvider
{
	GENERATED_BODY()

public:
	/**
	 * Retrieve the A2F curveWeights for the upcoming update tick.
	 *
	 * @param targetArrayRef The array that will be used to fill the new curves. Old values will be overwritten.
	 */
	virtual void GetA2FCurveWeightsPreUpdate(TArray<float>& targetArrayRef) {}
};
