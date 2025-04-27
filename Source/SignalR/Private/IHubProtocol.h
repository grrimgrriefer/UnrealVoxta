/*
 * MIT License
 *
 * Copyright (c) 2020-2022 Frozen Storm Interactive, Yoann Potinet
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "CoreMinimal.h"
#include "MessageType.h"
#include "SignalRValue.h"

/**
 * Base message structure for SignalR hub communication.
 * All specific message types derive from this base structure.
 */
struct FHubMessage
{
protected:
	FHubMessage(ESignalRMessageType InMessageType) :
		MessageType(InMessageType)
	{}

public:
	virtual ~FHubMessage() {}

	const ESignalRMessageType MessageType;
};

/**
 * Base message structure for invocation-related messages.
 * Contains common properties for messages that reference an invocation ID.
 */
struct FBaseInvocationMessage : FHubMessage
{
protected:
	FBaseInvocationMessage(const FString& InInvocationId, ESignalRMessageType InMessageType) : FHubMessage(InMessageType),
		InvocationId(InInvocationId)
	{}

public:
	const FString InvocationId;
};

/**
 * Represents a method invocation message sent from client to server.
 * Contains the target method name, arguments, and optional streaming IDs.
 */
struct FInvocationMessage : FBaseInvocationMessage
{
	FInvocationMessage(const FString& InInvocationId, const FString& InTarget, const TArray<FSignalRValue>& InArgs, const TArray<FString>& InStreamIds = TArray<FString>()) :
		FBaseInvocationMessage(InInvocationId, ESignalRMessageType::Invocation),
		Target(InTarget),
		Arguments(InArgs),
		StreamIds(InStreamIds)
	{}

	FInvocationMessage(FString&& InInvocationId, FString&& InTarget, TArray<FSignalRValue>&& InArgs, TArray<FString>&& InStreamIds = TArray<FString>()) :
		FBaseInvocationMessage(MoveTemp(InInvocationId), ESignalRMessageType::Invocation),
		Target(MoveTemp(InTarget)),
		Arguments(MoveTemp(InArgs)),
		StreamIds(MoveTemp(InStreamIds))
	{}

	FString Target;
	TArray<FSignalRValue> Arguments;
	TArray<FString> StreamIds;
};

/**
 * Represents a completion message received from the server after a method invocation.
 * Contains the result or error information for the completed method.
 */
struct FCompletionMessage : FBaseInvocationMessage
{
	FCompletionMessage(const FString& InInvocationId, const FString& InError, const FSignalRValue& InResult, bool InHasResult) :
		FBaseInvocationMessage(InInvocationId, ESignalRMessageType::Completion),
		Error(InError),
		HasResult(InHasResult),
		Result(InResult)
	{}

	FCompletionMessage(FString&& InInvocationId, FString&& InError, FSignalRValue&& InResult, bool InHasResult) :
		FBaseInvocationMessage(InInvocationId, ESignalRMessageType::Completion),
		Error(InError),
		HasResult(InHasResult),
		Result(InResult)
	{}

	FString Error;
	bool HasResult;
	FSignalRValue Result;
};

/**
 * Represents a ping message used to keep the connection alive.
 */
struct FPingMessage : FHubMessage
{
	FPingMessage() : FHubMessage(ESignalRMessageType::Ping)
	{}
};

/**
 * Represents a close message indicating the intention to close the connection.
 * May contain an error message and flag indicating if reconnection is allowed.
 */
struct FCloseMessage : FHubMessage
{
	FCloseMessage() : FHubMessage(ESignalRMessageType::Close)
	{}

	TOptional<FString> Error;
	TOptional<bool> bAllowReconnect;
};

/**
 * Interface for SignalR hub protocol implementations.
 * Defines methods for serializing and parsing SignalR messages.
 */
class IHubProtocol
{
public:
	virtual ~IHubProtocol();

	/**
	 * Gets the name of the protocol.
	 *
	 * @return The name of the protocol.
	 */
	virtual FName Name() const = 0;

	/**
	 * Gets the version of the protocol.
	 *
	 * @return The version of the protocol.
	 */
	virtual int Version() const = 0;

	/**
	 * Serializes a hub message to a string.
	 *
	 * @param Message The message to serialize.
	 
	 * @return The serialized message.
	 */
	virtual FString SerializeMessage(const FHubMessage*) const = 0;

	/**
	 * Parses a string containing one or more serialized hub messages.
	 *
	 * @param Message The string to parse.

	 * @return An array of parsed hub messages.
	 */
	virtual TArray<TSharedPtr<FHubMessage>> ParseMessages(const FString&) const = 0;
};
