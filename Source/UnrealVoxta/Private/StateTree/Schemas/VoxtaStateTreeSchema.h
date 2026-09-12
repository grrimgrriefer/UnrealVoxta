// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeExecutionTypes.h"
#include "StateTreeSchema.h"
#include "VoxtaStateTreeSchema.generated.h"

/**
 * Custom schema for the Voxta statetree
 */
UCLASS()
class UNREALVOXTA_API UVoxtaStateTreeSchema : public UStateTreeSchema
{
	GENERATED_BODY()

public:
	static const FName VOXTA_API_HANDLER_NAME;
	static const FName VOXTA_STATE_TREE_SUBSYSTEM_NAME;

	UVoxtaStateTreeSchema();

	virtual TConstArrayView<FStateTreeExternalDataDesc> GetContextDataDescs() const override;
	virtual bool IsStructAllowed(const UScriptStruct* inScriptStruct) const override;
	virtual bool IsExternalItemAllowed(const UStruct& inStruct) const override;

protected:
	UPROPERTY()
	FStateTreeExternalDataDesc m_voxtaApiHandlerData;
	UPROPERTY()
	FStateTreeExternalDataDesc m_voxtaSubsystemData;

	TArray<FStateTreeExternalDataDesc> m_contextDescs;
};
