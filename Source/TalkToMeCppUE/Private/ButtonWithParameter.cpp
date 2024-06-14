// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "ButtonWithParameter.h"
#include <Logging/StructuredLog.h>

void UButtonWithParameter::Initialize(FString value)
{
	OnClicked.AddDynamic(this, &UButtonWithParameter::OnClick);
	m_parameterValue = value;
}

void UButtonWithParameter::OnClick()
{
	OnClickedWithParam.Broadcast(m_parameterValue);
}