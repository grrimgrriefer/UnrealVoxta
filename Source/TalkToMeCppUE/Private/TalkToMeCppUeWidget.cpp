// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TalkToMeCppUeWidget.h"

void UTalkToMeCppUeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemTitle)
	{
		ItemTitle->SetText(FText::FromString(TEXT("Hello world!")));
	}
}