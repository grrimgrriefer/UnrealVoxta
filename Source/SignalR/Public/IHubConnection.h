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
#include "SignalRValue.h"

/**
 * Represents the result of a SignalR method invocation, containing either a value or error information.
 */
class FSignalRInvokeResult : public FSignalRValue
{
	friend struct FSignalRInvokeResultWrapper;
public:
	FSignalRInvokeResult(const FSignalRValue& Value) : FSignalRValue(Value) {}

	FSignalRInvokeResult(const FSignalRInvokeResult& OtherValue) : FSignalRValue(OtherValue)
	{
		bError = OtherValue.bError;
		ErrorMessage = OtherValue.ErrorMessage;
	}

	FSignalRInvokeResult(FSignalRInvokeResult&& OtherValue) noexcept : FSignalRValue(MoveTemp(OtherValue))
	{
		bError = MoveTemp(OtherValue.bError);
		ErrorMessage = MoveTemp(OtherValue.ErrorMessage);
	}

	FSignalRInvokeResult& operator=(const FSignalRInvokeResult& OtherValue)
	{
		FSignalRValue::operator=(OtherValue);
		bError = OtherValue.bError;
		ErrorMessage = OtherValue.ErrorMessage;
		return *this;
	}

	FSignalRInvokeResult& operator=(FSignalRInvokeResult&& OtherValue) noexcept
	{
		FSignalRValue::operator=(MoveTemp(OtherValue));
		bError = MoveTemp(OtherValue.bError);
		ErrorMessage = MoveTemp(OtherValue.ErrorMessage);
		return *this;
	}

	/**
	 * Checks if the invoke result contains an error.
	 *
	 * @return True if this result represents an error, false otherwise.
	 */
	FORCEINLINE bool HasError() const
	{
		return bError;
	}

	/** @return The error message associated with this result. */
	FORCEINLINE const FString& GetErrorMessage() const
	{
		return ErrorMessage;
	}

	/**
	 * Creates an error result with the specified message.

	 * @param ErrorMessage The error message to include in the result.

	 * @return A FSignalRInvokeResult instance representing an error.
	 */
	FORCEINLINE static FSignalRInvokeResult Error(const FString& ErrorMessage)
	{
		FSignalRInvokeResult Result;
		Result.bError = true;
		Result.ErrorMessage = ErrorMessage;
		return Result;
	}

private:
	FSignalRInvokeResult() = default;

	bool bError = false;
	FString ErrorMessage;
};

/**
 * Interface for a SignalR hub connection that enables real-time communication with a SignalR server.
 * Provides methods for starting/stopping connections, registering event handlers, and invoking server methods.
 */
class IHubConnection : public TSharedFromThis<IHubConnection>
{
public:
	/**
	 * Starts the connection to the SignalR hub.
	 */
	virtual void Start() = 0;

	/**
	 * Stops the connection to the SignalR hub.
	 */
	virtual void Stop() = 0;

	/**
	 * Delegate called when a connection has been established successfully.
	 */
	DECLARE_EVENT(IHubConnection, FOnHubConnectedEvent);

	/**
	 * Gets the event that is triggered when the connection is successfully established.
	 *
	 * @return Reference to the connection established event.
	 */
	virtual FOnHubConnectedEvent& OnConnected() = 0;

	/**
	 * Delegate called when a connection could not be established.
	 */
	DECLARE_EVENT_OneParam(IHubConnection, FOnHubConnectionErrorEvent, const FString& /* Error */);

	/**
	 * Gets the event that is triggered when a connection error occurs.
	 *
	 * @return Reference to the connection error event.
	 */
	virtual FOnHubConnectionErrorEvent& OnConnectionError() = 0;

	/**
	 * Delegate called when a web socket connection has been closed.
	 *
	 */
	DECLARE_EVENT(IHubConnection, FHubConnectionClosedEvent);

	/**
	 * Gets the event that is triggered when the connection is closed.
	 *
	 * @return Reference to the connection closed event.
	 */
	virtual FHubConnectionClosedEvent& OnClosed() = 0;

	DECLARE_DELEGATE_OneParam(FOnMethodInvocation, const TArray<FSignalRValue>&);

	/**
	 * Registers a callback for a specific hub method invocation from the server.
	 *
	 * @param EventName The name of the hub method to listen for.
	 *
	 * @return A delegate that will be invoked when the specified method is called by the server.
	 */
	virtual FOnMethodInvocation& On(const FString& EventName) = 0;

	DECLARE_DELEGATE_OneParam(FOnMethodCompletion, const FSignalRInvokeResult&);

	/**
	 * Invokes a hub method on the server with the specified arguments and waits for a response.
	 *
	 * @param EventName The name of the hub method to invoke.
	 * @param InArguments Array of arguments to pass to the hub method.
	 *
	 * @return A delegate that will be invoked when the server responds to the method invocation.
	 */
	virtual FOnMethodCompletion& Invoke(const FString& EventName, const TArray<FSignalRValue>& InArguments = TArray<FSignalRValue>()) = 0;

	/**
	 * Templated version of Invoke that converts arguments to FSignalRValue automatically.
	 *
	 * @tparam ArgTypes Variadic template parameters representing the argument types.
	 *                  Must be types that can be converted to FSignalRValue.
	 * @param EventName The name of the hub method to invoke.
	 * @param Arguments Variable number of arguments to pass to the hub method.
	 *
	 * @return A delegate that will be invoked when the server responds to the method invocation.
	 */
	template <typename... ArgTypes>
	FORCEINLINE FOnMethodCompletion& Invoke(const FString& EventName, ArgTypes... Arguments)
	{
		static_assert(TAnd<TIsConstructible<FSignalRValue, ArgTypes>...>::Value, "Invalid argument type passed to IHubConnection::Invoke");
		return Invoke(EventName, TArray<FSignalRValue> { MoveTemp(Arguments)... });
	}

	/**
	 * Sends a message to a hub method on the server without waiting for a response.
	 *
	 * @param EventName The name of the hub method to send a message to.
	 * @param InArguments Array of arguments to pass to the hub method.
	 */
	virtual void Send(const FString& EventName, const TArray<FSignalRValue>& InArguments = TArray<FSignalRValue>()) = 0;

	/**
	 * Templated version of Send that converts arguments to FSignalRValue automatically.
	 *
	 * @param EventName The name of the hub method to send a message to.
	 * @param Arguments Variable number of arguments to pass to the hub method.
	 */
	template <typename... ArgTypes>
	FORCEINLINE void Send(const FString& EventName, ArgTypes... Arguments)
	{
		static_assert(TAnd<TIsConstructible<FSignalRValue, ArgTypes>...>::Value, "Invalid argument type passed to IHubConnection::Send");
		Send(EventName, TArray<FSignalRValue> { MoveTemp(Arguments)... });
	}

protected:
	virtual ~IHubConnection();
};

typedef TSharedPtr<IHubConnection> IHubConnectionPtr;
