// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TalkToMeCppUeWidget.h"
#include "ButtonWithParameter.h"
#include "Logging/StructuredLog.h"
#include "VoxtaAudioPlayback.h"
#include "VoxtaAudioInput.h"

void UTalkToMeCppUeWidget::InitializeWidget(UVoxtaAudioPlayback* playbackHandler, UVoxtaAudioInput* inputHandler)
{
	m_playbackHandler = playbackHandler;
	m_inputHandler = inputHandler;
	UserInputField->OnTextCommitted.AddUniqueDynamic(this, &UTalkToMeCppUeWidget::OnUserInputCommittedInternal);
	UserInputField->SetIsEnabled(false);
}

void UTalkToMeCppUeWidget::CleanupWidget()
{
	if (CharScrollBox && UserInputField)
	{
		TArray<UWidget*> children = CharScrollBox->GetAllChildren();
		for (UWidget* child : children)
		{
			if (UButtonWithParameter* button = Cast<UButtonWithParameter>(child))
			{
				button->ClickedWithParamEvent.RemoveDynamic(this, &UTalkToMeCppUeWidget::OnCharacterButtonClickedInternal);
			}
		}
		UserInputField->OnTextCommitted.RemoveDynamic(this, &UTalkToMeCppUeWidget::OnUserInputCommittedInternal);
	}
	else
	{
		UE_LOGFMT(LogCore, Warning, "Failed to cleanup  UTalkToMeCppUeWidget, as the class was null.");
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
	if (UserInputField)
	{
		bool enabled = newState == VoxtaClientState::WaitingForUser ||
			newState == VoxtaClientState::AudioPlayback;
		UserInputField->SetIsEnabled(enabled);
		if (!enabled)
		{
			MicTempTranscription->SetText(FText::GetEmpty());
		}
	}
}

void UTalkToMeCppUeWidget::RegisterCharacterOption(const FAiCharData& charData)
{
	if (CharScrollBox)
	{
		UTextBlock* textBlock = NewObject<UTextBlock>(UTextBlock::StaticClass());
		textBlock->SetText(FText::FromStringView(charData.GetName()));
		textBlock->SetShadowColorAndOpacity(FLinearColor::Black);
		textBlock->SetShadowOffset(FVector2D(2));

		UButtonWithParameter* characterButton = NewObject<UButtonWithParameter>(this, UButtonWithParameter::StaticClass());
		characterButton->AddChild(textBlock);

		characterButton->Initialize(charData.GetId());
		characterButton->ClickedWithParamEvent.AddUniqueDynamic(this, &UTalkToMeCppUeWidget::OnCharacterButtonClickedInternal);
		CharScrollBox->AddChild(characterButton);
	}
}

void UTalkToMeCppUeWidget::AddTextMessage(const FCharDataBase& sender, FStringView messageId, FStringView message)
{
	if (ChatLogScrollBox)
	{
		UTextBlock* textBlock = NewObject<UTextBlock>(UTextBlock::StaticClass());
		textBlock->SetText(FText::FromString(FString::Format(*API_STRING("{0}: {1}"), {
			sender.GetName(),
			message
			})));

		textBlock->SetShadowColorAndOpacity(FLinearColor::Black);
		textBlock->SetShadowOffset(FVector2D(2));
		textBlock->SetAutoWrapText(true);

		m_messages.Add(FString(messageId), textBlock);
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

void UTalkToMeCppUeWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
	if (MicVolume && m_inputHandler)
	{
		MicVolume->SetPercent(m_inputHandler->GetNormalizedAmplitude() * 100.f);
	}
	if (MicIcon && m_inputHandler)
	{
		MicIcon->SetRenderOpacity(m_inputHandler->IsRecording() ? 1.f : 0.f);
		MicDeviceLabel->SetRenderOpacity(m_inputHandler->IsRecording() ? 1.f : 0.f);
	}
	if (PlaybackIcon && m_playbackHandler)
	{
		PlaybackIcon->SetRenderOpacity(m_playbackHandler->IsPlaying() ? 1.f : 0.f);
	}
	if (MicDeviceLabel && m_inputHandler && MicDeviceLabel->GetText().IsEmptyOrWhitespace())
	{
		MicDeviceLabel->SetText(FText::FromString(m_inputHandler->GetInputDeviceName()));
	}
}

void UTalkToMeCppUeWidget::PartialSpeechTranscription(const FString& message)
{
	MicTempTranscription->SetText(FText::FromString(message));
}

void UTalkToMeCppUeWidget::OnCharacterButtonClickedInternal(FString charId)
{
	CharButtonClickedEvent.Broadcast(charId);
}

void UTalkToMeCppUeWidget::OnUserInputCommittedInternal(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter && !Text.IsEmptyOrWhitespace())
	{
		UserInputCommittedEvent.Broadcast(Text.ToString());
		UserInputField->SetText(FText::GetEmpty());
	}
}