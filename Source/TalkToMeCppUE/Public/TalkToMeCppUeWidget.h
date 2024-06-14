// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "ButtonWithParameter.h"
#include "VoxtaClient.h"
#include "TalkToMeCppUeWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharButtonClickedSignature, FString, charID);

UCLASS(Abstract)
class TALKTOMECPPUE_API UTalkToMeCppUeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FCharButtonClickedSignature OnCharButtonClickedDelegate;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusLabel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> CharScrollBox;

	void UpdateLabelWithState(VoxtaClientState newState);
	void AddCharacterOption(const FCharData& charData);

protected:
	UFUNCTION()
	void SelectCharacter(FString charId);
};
