// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TalkToMeCppUeWidget.h"
#include "ButtonWithParameter.h"
#include "Logging/StructuredLog.h"

void UTalkToMeCppUeWidget::InitializeWidget()
{
	UserInputField->OnTextCommitted.AddUniqueDynamic(this, &UTalkToMeCppUeWidget::OnUserInputCommittedInternal);
	UserInputField->SetIsEnabled(false);
}

void UTalkToMeCppUeWidget::CleanupWidget()
{
	if (CharScrollBox)
	{
		TArray<UWidget*> children = CharScrollBox->GetAllChildren();
		for (UWidget* child : children)
		{
			if (UButtonWithParameter* button = Cast<UButtonWithParameter>(child))
			{
				button->OnClickedWithParam.RemoveAll(this);
			}
		}
	}
	if (UserInputField)
	{
		UserInputField->OnTextCommitted.RemoveAll(this);
	}
}

void UTalkToMeCppUeWidget::ConfigureWidgetForState(VoxtaClientState newState)
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
	if (UserInputField && newState == VoxtaClientState::Chatting)
	{
		UserInputField->SetIsEnabled(true);
	}
}

void UTalkToMeCppUeWidget::RegisterCharacterOption(const FCharData& charData)
{
	if (CharScrollBox)
	{
		UTextBlock* textBlock = NewObject<UTextBlock>(UTextBlock::StaticClass());
		textBlock->SetText(FText::FromString(charData.m_name));
		textBlock->SetShadowColorAndOpacity(FLinearColor::Black);
		textBlock->SetShadowOffset(FVector2D(2));

		UButtonWithParameter* characterButton = NewObject<UButtonWithParameter>(this, UButtonWithParameter::StaticClass());
		characterButton->AddChild(textBlock);

		characterButton->Initialize(charData.m_id);
		characterButton->OnClickedWithParam.AddUniqueDynamic(this, &UTalkToMeCppUeWidget::OnCharacterButtonClickedInternal);
		CharScrollBox->AddChild(characterButton);
	}
}

void UTalkToMeCppUeWidget::RegisterTextMessage(const FCharData& sender, const FString& messageId, const FString& message)
{
	if (ChatLogScrollBox)
	{
		UTextBlock* textBlock = NewObject<UTextBlock>(UTextBlock::StaticClass());
		textBlock->SetText(FText::FromString(FString::Format(*API_STRING("{0}: {1}"), {
			sender.m_name,
			message
			})));

		textBlock->SetShadowColorAndOpacity(FLinearColor::Black);
		textBlock->SetShadowOffset(FVector2D(2));
		textBlock->SetAutoWrapText(true);

		m_messages.Add(messageId, textBlock);
		ChatLogScrollBox->AddChild(textBlock);
	}
}

void UTalkToMeCppUeWidget::RemoveTextMessage(const FString& messageId)
{
	if (ChatLogScrollBox)
	{
		if (m_messages.Contains(messageId))
		{
			ChatLogScrollBox->RemoveChild(m_messages[messageId]);
		}
		m_messages.Remove(messageId);
	}
}

void UTalkToMeCppUeWidget::OnCharacterButtonClickedInternal(FString charId)
{
	OnCharButtonClickedEvent.Broadcast(charId);
}

void UTalkToMeCppUeWidget::OnUserInputCommittedInternal(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter && !Text.IsEmptyOrWhitespace())
	{
		OnUserInputCommittedEvent.Broadcast(Text.ToString());
		UserInputField->SetText(FText::GetEmpty());
	}
}