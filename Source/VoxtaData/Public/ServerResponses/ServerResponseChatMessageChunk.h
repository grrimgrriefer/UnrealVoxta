// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseChatMessageBase.h"

/// <summary>
/// Read-only data struct containing the relevant data of the 'replyChunk' response
/// from the VoxtaServer.
/// </summary>
struct ServerResponseChatMessageChunk : public IServerResponseChatMessageBase
{
public:
	const FString SENDER_ID;
	const int m_startIndex = 0;
	const int m_endIndex = 0;
	const FString m_messageText;
	const FString m_audioUrlPath;

	explicit ServerResponseChatMessageChunk(FStringView messageId,
			FStringView senderId,
			FStringView sessionId,
			int startIndex,
			int endIndex,
			FStringView messageText,
			FStringView audioUrlPath) :
		IServerResponseChatMessageBase(messageId, sessionId),
		SENDER_ID(senderId),
		m_startIndex(startIndex),
		m_endIndex(endIndex),
		m_messageText(messageText),
		m_audioUrlPath(audioUrlPath)
	{
	}

	/// <summary>
	/// Identifies the response type as MessageChunk.
	/// </summary>
	/// <returns>Returns MessageType::MessageChunk.</returns>
	MessageType GetMessageType() const final
	{
		return MessageType::MessageChunk;
	}
};