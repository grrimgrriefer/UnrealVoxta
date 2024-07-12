// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/EditableTextBox.h"
#include "VoxtaClient.h"
#include "Types/SlateEnums.h"
#include "TalkToMeCppUeWidget.generated.h"

class UVoxtaAudioPlayback;
class UVoxtaAudioInput;

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
	FCharButtonClickedEventCallback CharButtonClickedEvent;

	/// <summary>
	/// Event fired when the user has pressed ENTER after providing some text in the EditableTextBox
	/// </summary>
	UPROPERTY()
	FInputCommittedEventCallback UserInputCommittedEvent;

	/// <summary>
	/// Sets up the bindings for the inputfield and ensures the proper elements
	/// are enabled/disabled at the start.
	/// </summary>
	void InitializeWidget(UVoxtaAudioPlayback* playbackHandler, UVoxtaAudioInput* inputHandler);

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
	void AddTextMessage(const FCharDataBase& sender, FStringView messageId, FStringView message);

	/// <summary>
	/// If a chat message is found with a matching messageId,
	/// remove it from the chat log.
	/// </summary>
	/// <param name="messageId">The VoxtaServer messageId,
	/// that was provided to this widget when the message was sent.</param>
	void RemoveTextMessage(const FString& messageId);

	void PartialSpeechTranscription(const FString& message);

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

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> MicVolume;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> MicIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> PlaybackIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> MicDeviceLabel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> MicTempTranscription;
	///~ End Blueprint bindings.

	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

private:
	TMap<FString, UTextBlock*> m_messages;
	UVoxtaAudioPlayback* m_playbackHandler;
	UVoxtaAudioInput* m_inputHandler;

	/// <summary>
	/// Automatically invoked when one of the buttons tied to a character is clicked.
	/// It will in turn broadcast a click event with the charId as parameter of the event.
	/// </summary>
	/// <param name="charId">The FCharDataBase::m_id of the character tied to the button clicked.</param>
	UFUNCTION()
	void OnCharacterButtonClickedInternal(FString charId);

	/// <summary>
	/// Automatically invoked when the EditableTextBox::OnTextCommitted is triggered.
	/// This happens when the user presses enter or the text box loses focus.
	/// </summary>
	/// <param name="text">The text that the user had written in the textbox.</param>
	/// <param name="commitMethod">The type of trigger that caused this function to be invoked.</param>
	UFUNCTION()
	void OnUserInputCommittedInternal(const FText& text, ETextCommit::Type commitMethod);
};
