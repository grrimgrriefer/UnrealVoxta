// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TalkToMeCppUeWidget.h"

void UTalkToMeCppUeWidget::InitializeWidget()
{
	UserInputField->OnTextCommitted.AddUniqueDynamic(this, &UTalkToMeCppUeWidget::UserInputSubmitted);
}

void UTalkToMeCppUeWidget::UpdateLabelWithState(VoxtaClientState newState)
{
	if (StatusLabel)
	{
		StatusLabel->SetText(FText::FromName(StaticEnum<VoxtaClientState>()->GetNameByValue(StaticCast<int64>(newState))));
	}
	if (CharScrollBox && newState == VoxtaClientState::StartingChat)
	{
		TArray<UWidget*> children = CharScrollBox->GetAllChildren();
		for (UWidget* child : children)
		{
			if (UButtonWithParameter* button = Cast<UButtonWithParameter>(child))
			{
				button->SetIsEnabled(false);
			}
		}
	}
}

void UTalkToMeCppUeWidget::RegisterCharacterOption(const FCharData& charData)
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

void UTalkToMeCppUeWidget::RegisterTextMessage(const FCharData& sender, const FString& message)
{
	if (ChatLogScrollBox)
	{
		UTextBlock* textBlock = NewObject<UTextBlock>(UTextBlock::StaticClass());
		textBlock->SetText(FText::FromString(FString::Format(*API_STRING("{0}: {1}"), {
			sender.m_name,
			message
			})));

		textBlock->bWrapWithInvalidationPanel = true;

		ChatLogScrollBox->AddChild(textBlock);
	}
}

void UTalkToMeCppUeWidget::SelectCharacter(FString charId)
{
	OnCharButtonClickedDelegate.Broadcast(charId);
}

void UTalkToMeCppUeWidget::UserInputSubmitted(const FText& Text, ETextCommit::Type CommitType)
{
	OnUserInputFieldSubmittedDelegate.Broadcast(Text.ToString());
}