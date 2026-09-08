// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaStateTreeSchema.h"
#include "StateTreeConditionBase.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeTaskBase.h"
#include "RawAPI/VoxtaApiHandler.h"
#include "SubSystems/VoxtaStateTreeSubsystem.h"
#include "UObject/Package.h"

const FName UVoxtaStateTreeSchema::VOXTA_API_HANDLER_NAME = TEXT("VOXTA_API_HANDLER");
const FName UVoxtaStateTreeSchema::VOXTA_STATE_TREE_SUBSYSTEM_NAME = TEXT("VOXTA_STATE_TREE_SUBSYSTEM");

UVoxtaStateTreeSchema::UVoxtaStateTreeSchema() : m_socketHandlerData(VOXTA_API_HANDLER_NAME,
																	UVoxtaApiHandler::StaticClass(),
																	FGuid::NewDeterministicGuid(VOXTA_API_HANDLER_NAME.ToString())),
												m_voxtaSubsystemData(VOXTA_STATE_TREE_SUBSYSTEM_NAME,
																	UVoxtaStateTreeSubsystem::StaticClass(),
																	FGuid::NewDeterministicGuid(VOXTA_STATE_TREE_SUBSYSTEM_NAME.ToString()))
{
	m_socketHandlerData.Requirement = EStateTreeExternalDataRequirement::Required;
	m_voxtaSubsystemData.Requirement = EStateTreeExternalDataRequirement::Required;

	m_contextDescs = { m_socketHandlerData, m_voxtaSubsystemData };
}
TConstArrayView<FStateTreeExternalDataDesc> UVoxtaStateTreeSchema::GetContextDataDescs() const
{
	return m_contextDescs;
}
bool UVoxtaStateTreeSchema::IsStructAllowed(const UScriptStruct* inScriptStruct) const
{
	return inScriptStruct->IsChildOf(FStateTreeTaskBase::StaticStruct())
		|| inScriptStruct->IsChildOf(FStateTreeEvaluatorBase::StaticStruct())
		|| inScriptStruct->IsChildOf(FStateTreeConditionBase::StaticStruct());
}
bool UVoxtaStateTreeSchema::IsExternalItemAllowed(const UStruct& inStruct) const
{
	if (const UClass* itemClass = Cast<const UClass>(&inStruct))
	{
		return itemClass->IsChildOf(UVoxtaApiHandler::StaticClass())
			|| itemClass->IsChildOf(UVoxtaStateTreeSubsystem::StaticClass());
	}
	return false;
}
