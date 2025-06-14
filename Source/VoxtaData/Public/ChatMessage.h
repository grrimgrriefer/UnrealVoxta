// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ChatMessage.generated.h"

/**
 * FChatMessage
 * Represents a single message in a chat conversation, containing both text and audio data.
 * Messages can be built up incrementally as chunks are received from the server.
 * A message is considered complete only after being marked as such.
 */
USTRUCT(BlueprintType, Category = "Voxta")
struct FChatMessage
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Get the unique identifier assigned to this message.
	 * Used to track and reference this specific message.
	 *
	 * @return The message's unique ID.
	 */
	const FGuid& GetMessageId() const { return m_messageId; }

	/**
	 * Get the ID of the character who sent this message.
	 * Used to identify the sender of the message.
	 *
	 * @return The sender's character ID.
	 */
	const FGuid& GetCharId() const { return m_charId; }

	/**
	 * Get the current text content of the message.
	 * May be incomplete if message is still being received.
	 *
	 * @return The current message text.
	 */
	FStringView GetTextContent() const { return m_text; }

	/**
	 * Get the list of audio URLs for this message's synthesized speech.
	 * Used to retrieve audio content for playback.
	 *
	 * @return Array of audio URL strings.
	 */
	const TArray<FString>& GetAudioUrls() const { return m_audioUrls; }

	/**
	 * Check if this message is complete with no further chunks expected.
	 * Used to determine if message content is finalized.
	 *
	 * @return True if message is complete, false if still receiving chunks.
	 */
	bool GetIsComplete() const { return m_isComplete; }

	/**
	 * Add more data to this message, as VoxtaServer notifies us of the complete data in chunks.
	 *
	 * @param textContent The new text that will be appended to what we already had.
	 * @param audioUrl The new audio (sub)url that will be registered as required for playback.
	 */
	bool TryAppendMoreContent(const FString& textContent, const FString& audioUrl)
	{
		if (m_isComplete)
		{
			return false;
		}

		m_text.Append(textContent);
		if (!audioUrl.IsEmpty()) // text only response is valid, but we don't add empty audio urls ofc
		{
			m_audioUrls.Emplace(audioUrl);
		}
		return true;
	}

	/**
	 * Mark this message as complete, indicating no further chunks are expected.
	 */
	void MarkComplete()
	{
		m_isComplete = true;
	}

	/**
	 * Create an instance of the chat message for the User data.
	 * The notification by VoxtaServer of a new message is always without text, so it will be empty at first.
	 *
	 * @param messageId The VoxtaServer assigned id of this message.
	 * @param charId The VoxtaServer assigned id of the character that says this message.
	 */
	explicit FChatMessage(FGuid messageId, FGuid charId) :
		m_messageId(messageId),
		m_charId(charId)
	{}

	/** Default constructor. */
	FChatMessage() = default;
#pragma endregion

private:
	/** Message text (so far). */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Message Text (so far)"))
	FString m_text;

	/** Message ID assigned by VoxtaServer. */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Message ID"))
	FGuid m_messageId;

	/** Character ID of the sender. */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Character ID"))
	FGuid m_charId;

	/** Whether this message is complete. */
	bool m_isComplete = false;

	/** List of audio URLs for this message. */
	TArray<FString> m_audioUrls;
};