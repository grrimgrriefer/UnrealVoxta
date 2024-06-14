// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TalkToMeCppUeWidget.h"

void UTalkToMeCppUeWidget::UpdateLabelWithState(VoxtaClientState newState)
{
	if (StatusLabel)
	{
		StatusLabel->SetText(FText::FromName(StaticEnum<VoxtaClientState>()->GetNameByValue(StaticCast<int64>(newState))));
	}
}

void UTalkToMeCppUeWidget::AddCharacterOption(const FCharData& charData)
{
	if (CharScrollBox)
	{
		UTextBlock* textBlock = NewObject<UTextBlock>(UTextBlock::StaticClass());
		textBlock->SetText(FText::FromString(charData.m_name));

		UButton* testButton = NewObject<UButton>(this, UButton::StaticClass());
		testButton->AddChild(textBlock);

		CharScrollBox->AddChild(testButton);
	}
}