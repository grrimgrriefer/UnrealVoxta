// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "VoxtaClient.h"
#include "Types/SlateEnums.h"
#include "TalkToMeCppUeWidget.generated.h"

/// Declared as global since the ATalkToMeCppUeHUD class forwards these events and thus also
/// needs access to the delegate declarations.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharButtonClickedEventCallback, FString, charId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputCommittedEventCallback, FString, inputText);

/// <summary>
/// Widget class that contains the UI elements and handles the input & ouput to them.
/// This widget is Initialized from the ATalkToMeCppUeHUD, and should only be accessed through that class.
/// </summary>
UCLASS(Abstract)
class TALKTOMECPPUE_API UTalkToMeCppUeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/// <summary>
	/// Event fired when the user has clicked on a button linked to a specific character, with
	/// the intention of loading that character into a chat conversation.
	/// </summary>
	UPROPERTY()
	FCharButtonClickedEventCallback OnCharButtonClickedEvent;

	/// <summary>
	/// Event fired when the user has pressed ENTER after providing some text in the EditableTextBox
	/// </summary>
	UPROPERTY()
	FInputCommittedEventCallback OnUserInputCommittedEvent;

	/// <summary>
	/// Sets up the bindings for the inputfield and ensures the proper elements
	/// are enabled/disabled at the start.
	/// </summary>
	void InitializeWidget();

	/// <summary>
	/// Ensures that the bindings for the inputfield and any generated buttons are
	/// removed, so it can be safely deleted.
	/// </summary>
	void CleanupWidget();

	/// <summary>
	/// Ensures that the correct elements are enabled / disabled for the provided new state.
	/// Also ensures that the user-facing label is updated with the String representation of the
	/// provided VoxtaClientState.
	/// </summary>
	/// <param name="newState">The state for which you want this widget to configure itself for.</param>
	void ConfigureWidgetForState(VoxtaClientState newState);

	/// <summary>
	/// Adds a new user-facing selectable button to the character selection,
	/// also sets up the binding so OnCharButtonClickedEvent is triggered when the button is pressed.
	/// </summary>
	/// <param name="charData">The FCharData of the character that should be loaded when the button is pressed.</param>
	void RegisterCharacterOption(const FAiCharData& charData);

	/// <summary>
	/// Add a new text message to the bottom of the chat log.
	/// </summary>
	/// <param name="sender">The FCharData of the character that is says the message.</param>
	/// <param name="messageId">The VoxtaServer messageId, to keep track of edits and deletions.</param>
	/// <param name="message">The text of the message itself.</param>
	void RegisterTextMessage(const FCharDataBase& sender, FStringView messageId, FStringView message);

	/// <summary>
	/// If a chat message is found with a matching messageId,
	/// remove it from the chat log.
	/// </summary>
	/// <param name="messageId">The VoxtaServer messageId,
	/// that was provided to this widget when the message was sent.</param>
	void RemoveTextMessage(const FString& messageId);

	///~ Begin Blueprint bindings.
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusLabel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> CharScrollBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> ChatLogScrollBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEditableTextBox> UserInputField;
	///~ End Blueprint bindings.

private:
	TMap<FString, UTextBlock*> m_messages;

	UFUNCTION()
	void OnCharacterButtonClickedInternal(FString charId);

	UFUNCTION()
	void OnUserInputCommittedInternal(const FText& text, ETextCommit::Type commitMethod);
};
