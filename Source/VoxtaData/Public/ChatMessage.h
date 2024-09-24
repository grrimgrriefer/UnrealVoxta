// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ChatMessage.generated.h"

/**
 * Data struct representing all the relevant information for a single chat message.
 *
 * NOTE: The text and audioUrls are appended with new info as more chunks are received.
 */
USTRUCT(BlueprintType)
struct FChatMessage
{
	GENERATED_BODY()

#pragma region public API
public:
	/**  @return Immutable reference to the VoxtaServer assigned id of this message. */
	const FString& GetMessageId() const { return m_messageId; };

	/**  @return Immutable reference to the VoxtaServer assigned id of the character who said this message. */
	FStringView GetCharId() const { return m_charId; };

	/**  @return Immutable reference to the complete text (so far) of this message. */
	FStringView GetTextContent() const { return m_text; };

	/**
	 * @return Immutable reference to the list of audioUrl chunks that can be used to download the
	 * synthesized audio for all the text in this message.
	 */
	const TArray<FString>& GetAudioUrls() const { return m_audioUrls; };

	/**
	 * Add more data to this message, as VoxtaServer notifies us of the complete data in chunks.
	 *
	 * @param textContent The new text that will be appended to what we already had.
	 * @param audioUrl The new audio (sub)url that will be registered as required for playback.
	 */
	void AppendMoreContent(const FString& textContent, const FString& audioUrl)
	{
		m_text.Append(textContent);
		if (!audioUrl.IsEmpty()) // text only response is valid, but we dont'adde empty audio urls ofc
		{
			m_audioUrls.Emplace(audioUrl);
		}
	}

	/**
	 * Create an instance of the chat message for the User data.
	 * The notification by VoxtaServer of a new message is always without text, so it will be empty at first.
	 *
	 * @param messageId The VoxtaServer assigned id of this message.
	 * @param charId The VoxtasServer assigne id of the character that says this message.
	 */
	explicit FChatMessage(FStringView messageId, FStringView charId) :
		m_messageId(messageId),
		m_charId(charId)
	{
	}

	/** Default constructor, should not be used manually, but is enforced by Unreal */
	explicit FChatMessage() {};
#pragma endregion

private:
	// TODO: Add flag to indicate once the message is complete and no further chunks are expected.
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Message Text (so far)"))
	FString m_text;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Message ID"))
	FString m_messageId;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Character ID"))
	FString m_charId;

	TArray<FString> m_audioUrls;
};