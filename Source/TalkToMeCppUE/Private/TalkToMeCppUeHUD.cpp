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
		m_hudWidget->OnCharButtonClickedDelegate.AddUniqueDynamic(this, &ATalkToMeCppUeHUD::OnCharButtonClicked);
		m_hudWidget->OnUserInputFieldSubmittedDelegate.AddUniqueDynamic(this, &ATalkToMeCppUeHUD::OnUserInputFieldSubmitted);
	}

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	if (playerController)
	{
		playerController->bShowMouseCursor = true;
	}
}

void ATalkToMeCppUeHUD::VoxtaClientStateChanged(VoxtaClientState newState)
{
	m_hudWidget->UpdateLabelWithState(newState);
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

void ATalkToMeCppUeHUD::OnCharButtonClicked(FString charID)
{
	OnCharButtonClickedDelegate.Broadcast(charID);
}

void ATalkToMeCppUeHUD::OnUserInputFieldSubmitted(FString inputText)
{
	OnUserInputFieldSubmittedDelegate.Broadcast(inputText);
}