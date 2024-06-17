// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "ButtonWithParameter.h"
#include "VoxtaClient.h"
#include "Logging/StructuredLog.h"
#include "Types/SlateEnums.h"
#include "TalkToMeCppUeWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharButtonClickedSignature, FString, charID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputFieldSignature, FString, inputText);

UCLASS(Abstract)
class TALKTOMECPPUE_API UTalkToMeCppUeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FCharButtonClickedSignature OnCharButtonClickedDelegate;

	UPROPERTY()
	FInputFieldSignature OnUserInputFieldSubmittedDelegate;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusLabel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> CharScrollBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> ChatLogScrollBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserInputField;

	void InitializeWidget();

	void UpdateLabelWithState(VoxtaClientState newState);
	void RegisterCharacterOption(const FCharData& charData);
	void RegisterTextMessage(const FCharData& sender, const FString& message);

protected:
	UFUNCTION()
	void SelectCharacter(FString charId);

	UFUNCTION()
	void UserInputSubmitted(const FText& text, ETextCommit::Type commitMethod);
};
