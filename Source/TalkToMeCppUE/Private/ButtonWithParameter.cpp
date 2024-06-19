// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "ButtonWithParameter.h"
#include <Logging/StructuredLog.h>

void UButtonWithParameter::Initialize(FStringView value)
{
	OnClicked.AddDynamic(this, &UButtonWithParameter::OnClickInternal);
	m_parameterValue = value;
}

void UButtonWithParameter::BeginDestroy()
{
	Super::BeginDestroy();

	OnClicked.RemoveDynamic(this, &UButtonWithParameter::OnClickInternal);
}

void UButtonWithParameter::OnClickInternal()
{
	OnClickedWithParam.Broadcast(m_parameterValue);
}