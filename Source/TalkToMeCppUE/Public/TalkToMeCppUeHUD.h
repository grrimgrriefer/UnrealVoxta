// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TalkToMeCppUeWidget.h"
#include "TalkToMeCppUeHUD.generated.h"

/**
 *
 */
UCLASS()
class TALKTOMECPPUE_API ATalkToMeCppUeHUD : public AHUD
{
	GENERATED_BODY()

public:
	ATalkToMeCppUeHUD();

	class UClass* hudWidgetClass;
	class UTalkToMeCppUeWidget* hudWidget;

	virtual void BeginPlay() override;
};
