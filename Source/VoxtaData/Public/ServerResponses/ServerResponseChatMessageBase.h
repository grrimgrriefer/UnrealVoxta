// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/// <summary>
/// Abstract read-only data struct containing the shared relevant data for all
/// the 'replyX' responses from the VoxtaServer.
/// </summary>
struct IServerResponseChatMessageBase : ServerResponseBase
{
public:
	enum class MessageType
	{
		MessageStart,
		MessageChunk,
		MessageEnd,
		MessageCancelled
	};

	const FString MESSAGE_ID;
	const FString SESSION_ID;

	explicit IServerResponseChatMessageBase(
			FStringView messageId,
			FStringView sessionId) :
		MESSAGE_ID(messageId),
		SESSION_ID(sessionId)
	{
	}

	/// <summary>
	/// Const fuction that fetches the type of Message that the derived type represents.
	/// </summary>
	/// <returns>The type that identifies the derived response.</returns>
	virtual MessageType GetMessageType() const = 0;

	/// <summary>
	/// Identifies the response type as ChatMessage.
	/// </summary>
	/// <returns>Returns MessageType::ChatMessage.</returns>
	ServerResponseType GetType() const override
	{
		return ServerResponseType::ChatMessage;
	}
};