// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TalkToMeCppUeWidget.h"
#include "VoxtaClient.h"
#include "VoxtaData/Public/AiCharData.h"
#include "VoxtaData/Public/ChatMessage.h"
#include "TalkToMeCppUeHUD.generated.h"

/// <summary>
/// Class that manages the UTalkToMeCppUeWidget instance and handles all communication between
/// it and the VoxtaClient.
/// </summary>
UCLASS()
class TALKTOMECPPUE_API ATalkToMeCppUeHUD : public AHUD
{
	GENERATED_BODY()

public:
	ATalkToMeCppUeHUD();

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
	/// Notify the UTalkToMeCppUeWidget instance of the newly active state, causing it to configure itself
	/// for the provided state.
	/// Should only be invoked based on VoxtaClient events.
	/// </summary>
	/// <param name="newState">The new active state.</param>
	UFUNCTION()
	void VoxtaClientStateChanged(VoxtaClientState newState);

	/// <summary>
	/// Notify the UTalkToMeCppUeWidget of a new selectable Character being present for the user.
	/// Should only be invoked based on VoxtaClient events.
	/// </summary>
	/// <param name="charData">The FCharData of the newly selectable character.</param>
	UFUNCTION()
	void VoxtaClientCharacterRegistered(const FAiCharData& charData);

	/// <summary>
	/// Notify the UTalkToMeCppUeWidget of a new text message uttered by a character.
	/// Should only be invoked based on VoxtaClient events.
	///
	/// Note: This is used for both AI characters and for user messages.
	/// </summary>
	/// <param name="sender">The FCharData of the character that is says the message.</param>
	/// <param name="message">The FChatMessage containing all the relevant data of the message that has to
	/// be added to the log.</param>
	UFUNCTION()
	void AddTextMessage(const FCharDataBase& sender, const FChatMessage& message);

	/// <summary>
	/// Notify the UTalkToMeCppUeWidget of a specific a chat message being deleted.
	/// Should only be invoked based on VoxtaClient events.
	/// </summary>
	/// <param name="messageId">The VoxtaServer messageId, that was tied to the message when it was
	/// sent via the AddTextMessage function.</param>
	UFUNCTION()
	void RemoveTextMessage(const FChatMessage& message);

private:
	class UClass* m_hudWidgetClass;
	class UTalkToMeCppUeWidget* m_hudWidget;

	/// <summary>
	/// Automatically invoked when one of the buttons tied to a character is clicked.
	/// It will rebroadcast the event to a public API that can be used to bind external systems (like VoxtaClient) to.
	/// </summary>
	/// <param name="charId">The FCharDataBase::m_id of the character tied to the button clicked.</param>
	UFUNCTION()
	void OnCharButtonClicked(FString charId);

	/// <summary>
	/// Automatically invoked when the user has pressed ENTER after providing some text in the
	/// EditableTextBox of the widget.
	/// </summary>
	/// <param name="inputText">The text that the user had written in the textbox.</param>
	UFUNCTION()
	void OnUserInputFieldSubmitted(FString inputText);

	///~ Begin AHUD overrides.
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	///~ End AHUD bindings.
};
