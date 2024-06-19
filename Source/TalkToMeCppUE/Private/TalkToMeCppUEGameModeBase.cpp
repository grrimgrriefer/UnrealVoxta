// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TalkToMeCppUEGameModeBase.h"
#include "TestGameCharacter.h"
#include "TalkToMeCppUeHUD.h"

ATalkToMeCppUEGameModeBase::ATalkToMeCppUEGameModeBase()
{
	DefaultPawnClass = ATestGameCharacter::StaticClass();
	HUDClass = ATalkToMeCppUeHUD::StaticClass();
}

void ATalkToMeCppUEGameModeBase::StartPlay()
{
	Super::StartPlay();

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	if (playerController)
	{
		playerController->bShowMouseCursor = true;
	}
	else
	{
		UE_LOGFMT(LogCore, Warning, "Failed to find the PlayerController, so the GameMode couldn't make the cursor visible.");
	}
}