// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TalkToMeCppUeHUD.h"

ATalkToMeCppUeHUD::ATalkToMeCppUeHUD()
{
	static ConstructorHelpers::FClassFinder<UTalkToMeCppUeWidget> hudWidgetObj(TEXT("/Game/UI/BP_UI"));
	if (hudWidgetObj.Succeeded())
	{
		m_hudWidgetClass = hudWidgetObj.Class;
	}
	else
	{
		m_hudWidgetClass = nullptr;
	}
}

void ATalkToMeCppUeHUD::BeginPlay()
{
	Super::BeginPlay();
	if (m_hudWidgetClass)
	{
		m_hudWidget = CreateWidget<UTalkToMeCppUeWidget>(this->GetOwningPlayerController(), this->m_hudWidgetClass);
		m_hudWidget->AddToViewport();

		m_hudWidget->InitializeWidget();
		m_hudWidget->OnCharButtonClickedEvent.AddUniqueDynamic(this, &ATalkToMeCppUeHUD::OnCharButtonClicked);
		m_hudWidget->OnUserInputCommittedEvent.AddUniqueDynamic(this, &ATalkToMeCppUeHUD::OnUserInputFieldSubmitted);
	}
	else
	{
		UE_LOGFMT(LogCore, Error, "Failed to instance UTalkToMeCppUeWidget, as the class was null.");
	}
}

void ATalkToMeCppUeHUD::VoxtaClientStateChanged(VoxtaClientState newState)
{
	m_hudWidget->ConfigureWidgetForState(newState);
}

void ATalkToMeCppUeHUD::VoxtaClientCharacterLoaded(const FCharData& charData)
{
	m_hudWidget->RegisterCharacterOption(charData);
}

void ATalkToMeCppUeHUD::RegisterTextMessage(const FCharData& sender, const FChatMessage& message)
{
	m_hudWidget->RegisterTextMessage(sender, message.m_messageId, message.m_text);
}

void ATalkToMeCppUeHUD::RemoveTextMessage(const FChatMessage& message)
{
	m_hudWidget->RemoveTextMessage(message.m_messageId);
}

void ATalkToMeCppUeHUD::OnCharButtonClicked(FString charId)
{
	OnCharButtonClickedEvent.Broadcast(charId);
}

void ATalkToMeCppUeHUD::OnUserInputFieldSubmitted(FString inputText)
{
	OnUserInputCommittedEvent.Broadcast(inputText);
}