// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "StateTreeContextBinder.h"
#include "StateTreeExecutionContext.h"

bool StateTreeContextBinder::TryBindContextData(UObject* data)
{
	if (!IsValid(data))
	{
		return false;
	}

	m_contextObjects.RemoveAll([data](const TWeakObjectPtr<UObject>& weakObj)
	{
		if (!weakObj.IsValid())
		{
			return true;
		}
		const UObject* obj = weakObj.Get();
		return !obj || obj->GetClass() == data->GetClass();
	});

	m_contextObjects.AddUnique(data);
	return true;
}
bool StateTreeContextBinder::TryUnbindContextData(UObject* data)
{
	if (!IsValid(data))
	{
		return false;
	}

	const int32 removedCount = m_contextObjects.RemoveAll([data](const TWeakObjectPtr<UObject>& weakObj)
	{
		return !weakObj.IsValid() || weakObj.Get() == data;
	});

	return removedCount > 0;
}
bool StateTreeContextBinder::SetContextRequirements(FStateTreeExecutionContext& context, const UStateTree* stateTreeAsset, UObject* owner)
{
	if (!context.IsValid() || !stateTreeAsset)
	{
		return false;
	}

	const UStateTreeSchema* schema = stateTreeAsset->GetSchema();
	if (!schema)
	{
		return false;
	}

	for (const FStateTreeExternalDataDesc& desc : schema->GetContextDataDescs())
	{
		if (const UClass* expectedClass = Cast<const UClass>(desc.Struct))
		{
			UObject* matchedObject = nullptr;
			if (owner && owner->IsA(expectedClass))
			{
				matchedObject = owner;
			}
			else
			{
				for (const TWeakObjectPtr<UObject>& weakObj : m_contextObjects)
				{
					if (UObject* obj = weakObj.Get())
					{
						if (obj->IsA(expectedClass))
						{
							matchedObject = obj;
							break;
						}
					}
				}
			}

			if (matchedObject)
			{
				context.SetContextDataByName(desc.Name, FStateTreeDataView(desc.Struct, matchedObject));
			}
		}
	}
	context.SetCollectExternalDataCallback(FOnCollectStateTreeExternalData::CreateRaw(this, &StateTreeContextBinder::CollectExternalData, owner));

	return context.AreContextDataViewsValid();
}
bool StateTreeContextBinder::CollectExternalData(
	const FStateTreeExecutionContext& context,
	const UStateTree* stateTree,
	TArrayView<const FStateTreeExternalDataDesc> externalDataDescs,
	TArrayView<FStateTreeDataView> outDataViews,
	UObject* owner)
{
	bool allRequirementsMet = true;

	for (int32 i = 0; i < externalDataDescs.Num(); ++i)
	{
		const FStateTreeExternalDataDesc& desc = externalDataDescs[i];

		if (const UClass* targetClass = Cast<const UClass>(desc.Struct))
		{
			UObject* matchedObject = nullptr;
			if (owner && owner->IsA(targetClass))
			{
				matchedObject = owner;
			}
			else
			{
				for (const TWeakObjectPtr<UObject>& weakObj : m_contextObjects)
				{
					if (UObject* obj = weakObj.Get())
					{
						if (obj->IsA(targetClass))
						{
							matchedObject = obj;
							break;
						}
					}
				}
			}
			outDataViews[i] = FStateTreeDataView(matchedObject);
		}

		if (desc.Requirement == EStateTreeExternalDataRequirement::Required && !outDataViews[i].IsValid())
		{
			UE_LOG(LogStateTree,
					Error,
					TEXT("%s: Failed to provide required external data: %s"),
					owner ? *owner->GetName() : TEXT("ContextBinder"),
					*desc.Struct->GetName());
			allRequirementsMet = false;
		}
	}

	return allRequirementsMet;
}
