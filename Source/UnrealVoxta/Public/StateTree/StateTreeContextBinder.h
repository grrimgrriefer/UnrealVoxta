// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeExecutionContext.h"

class UStateTree;

/**
 * Helper class to manage dynamic context objects and bind context/external data for StateTree execution.
 */
class UNREALVOXTA_API StateTreeContextBinder
{
public:
	bool TryBindContextData(UObject* data);
	bool TryUnbindContextData(UObject* data);
	bool SetContextRequirements(FStateTreeExecutionContext& context, const UStateTree* stateTreeAsset, UObject* owner);

private:
	bool CollectExternalData(
		const FStateTreeExecutionContext& context,
		const UStateTree* stateTree,
		TArrayView<const FStateTreeExternalDataDesc> externalDataDescs,
		TArrayView<FStateTreeDataView> outDataViews,
		UObject* owner);

	TArray<TWeakObjectPtr<UObject>> m_contextObjects;
};
