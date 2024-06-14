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

		UButtonWithParameter* testButton = NewObject<UButtonWithParameter>(this, UButtonWithParameter::StaticClass());
		testButton->AddChild(textBlock);

		testButton->Initialize(charData.m_id);
		testButton->OnClickedWithParam.AddUniqueDynamic(this, &UTalkToMeCppUeWidget::SelectCharacter);
		CharScrollBox->AddChild(testButton);
	}
}

void UTalkToMeCppUeWidget::SelectCharacter(FString charId)
{
	OnCharButtonClickedDelegate.Broadcast(charId);
}