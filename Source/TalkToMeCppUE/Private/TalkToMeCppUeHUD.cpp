// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TalkToMeCppUeHUD.h"

ATalkToMeCppUeHUD::ATalkToMeCppUeHUD()
{
	// notice that at this point we can't guarantee that the playerController is actually constructed yet, so we can't get a reference to it
	static ConstructorHelpers::FClassFinder<UTalkToMeCppUeWidget> hudWidgetObj(TEXT("/Game/UI/BP_UI"));
	if (hudWidgetObj.Succeeded())
	{
		hudWidgetClass = hudWidgetObj.Class;
	}
	else
	{
		// hudWidgetObj not found
		hudWidgetClass = nullptr;
	}
}

void ATalkToMeCppUeHUD::BeginPlay()
{
	Super::BeginPlay();
	if (hudWidgetClass)
	{
		hudWidget = CreateWidget<UTalkToMeCppUeWidget>(this->GetOwningPlayerController(), this->hudWidgetClass);
		hudWidget->AddToViewport();
	}
}