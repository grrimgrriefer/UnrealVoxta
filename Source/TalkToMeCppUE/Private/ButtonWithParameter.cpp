// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "ButtonWithParameter.h"
#include <Logging/StructuredLog.h>

void UButtonWithParameter::Initialize(FString value)
{
	OnClicked.AddDynamic(this, &UButtonWithParameter::OnClickInternal);
	m_parameterValue = value;
}

void UButtonWithParameter::BeginDestroy()
{
	Super::BeginDestroy();

	// Unbind the delegate to prevent it from being called after the object is destroyed.
	OnClicked.RemoveDynamic(this, &UButtonWithParameter::OnClickInternal);
}

void UButtonWithParameter::OnClickInternal()
{
	OnClickedWithParam.Broadcast(m_parameterValue);
}