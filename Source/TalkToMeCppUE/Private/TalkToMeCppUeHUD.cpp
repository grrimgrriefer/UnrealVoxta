// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TalkToMeCppUeHUD.h"

ATalkToMeCppUeHUD::ATalkToMeCppUeHUD()
{
	// TODO: turn this into blueprint assignable value
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
		UE_LOGFMT(LogCore, Error, "Failed to instantiate UTalkToMeCppUeWidget, as the class was null.");
	}
}

void ATalkToMeCppUeHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (m_hudWidget)
	{
		m_hudWidget->OnCharButtonClickedEvent.RemoveDynamic(this, &ATalkToMeCppUeHUD::OnCharButtonClicked);
		m_hudWidget->OnUserInputCommittedEvent.RemoveDynamic(this, &ATalkToMeCppUeHUD::OnUserInputFieldSubmitted);

		OnCharButtonClickedEvent.Clear();
		OnUserInputCommittedEvent.Clear();

		m_hudWidget->RemoveFromParent();
		m_hudWidget = nullptr;
	}
	else
	{
		UE_LOGFMT(LogCore, Error, "Failed to instantiate UTalkToMeCppUeWidget, as the class was null.");
	}
}

void ATalkToMeCppUeHUD::VoxtaClientStateChanged(VoxtaClientState newState)
{
	m_hudWidget->ConfigureWidgetForState(newState);
}

void ATalkToMeCppUeHUD::VoxtaClientCharacterLoaded(const FAiCharData& charData)
{
	m_hudWidget->RegisterCharacterOption(charData);
}

void ATalkToMeCppUeHUD::RegisterTextMessage(const FCharDataBase& sender, const FChatMessage& message)
{
	m_hudWidget->RegisterTextMessage(sender, message.GetMessageId(), message.m_text);
}

void ATalkToMeCppUeHUD::RemoveTextMessage(const FChatMessage& message)
{
	m_hudWidget->RemoveTextMessage(FString(message.GetMessageId()));
}

void ATalkToMeCppUeHUD::OnCharButtonClicked(FString charId)
{
	OnCharButtonClickedEvent.Broadcast(charId);
}

void ATalkToMeCppUeHUD::OnUserInputFieldSubmitted(FString inputText)
{
	OnUserInputCommittedEvent.Broadcast(inputText);
}