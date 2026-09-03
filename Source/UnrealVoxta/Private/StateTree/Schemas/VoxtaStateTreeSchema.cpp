// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaStateTreeSchema.h"
#include "StateTreeConditionBase.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeTaskBase.h"
#include "VoxtaSocketHandler.h"
#include "UObject/Package.h"

const FName UVoxtaStateTreeSchema::m_SocketHandlerName = TEXT("Subsystem");

UVoxtaStateTreeSchema::UVoxtaStateTreeSchema() : m_socketHandlerData(m_SocketHandlerName,
																UVoxtaSocketHandler::StaticClass(),
																FGuid::NewDeterministicGuid(m_SocketHandlerName.ToString()))
{
	m_contextDescs = { m_socketHandlerData };
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
		return itemClass->IsChildOf(UVoxtaSocketHandler::StaticClass());
	}
	return false;
}
