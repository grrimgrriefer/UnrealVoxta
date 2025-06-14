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

#include "CallbackManager.h"
#include "CoreMinimal.h"
#include "IHubConnection.h"
#include "IHubProtocol.h"
#include "Tickable.h"

class FConnection;

/**
 * Implements a connection to a SignalR hub.
 * Provides methods to start and stop the connection, invoke hub methods,
 * and register handlers for hub method invocations.
 */
class FHubConnection : public IHubConnection, FTickableGameObject
{
public:
	static constexpr float PingTimer = 10.0f;

	/**
	 * Creates a new connection to a SignalR hub.
	 *
	 * @param InUrl The URL of the SignalR hub.
	 * @param InHeaders HTTP headers to include in requests to the server.
	 */
	FHubConnection(const FString& InUrl, const TMap<FString, FString>& InHeaders);

	/**
	 * Destructor for the hub connection.
	 * Cleans up resources and closes the connection if necessary.
	 */
	virtual ~FHubConnection();

#pragma region IHubConnection overrides
public:
	/**
	 * Starts the hub connection.
	 * Initiates the connection process to the SignalR hub.
	 */
	virtual void Start() override;

	/**
	 * Stops the hub connection.
	 * Closes the connection to the SignalR hub and cleans up resources.
	 */
	virtual void Stop() override;

	/**
	 * Gets the event that is triggered when the hub connection is established.
	 *
	 * @return Reference to the hub connected event.
	 */
	FORCEINLINE virtual FOnHubConnectedEvent& OnConnected() override
	{
		return OnHubConnectedEvent;
	}

	/**
	 * Gets the event that is triggered when a hub connection error occurs.
	 *
	 * @return Reference to the hub connection error event.
	 */
	FORCEINLINE virtual FOnHubConnectionErrorEvent& OnConnectionError() override
	{
		return OnHubConnectionErrorEvent;
	}

	/**
	 * Gets the event that is triggered when the hub connection is closed.
	 *
	 * @return Reference to the hub connection closed event.
	 */
	FORCEINLINE virtual FHubConnectionClosedEvent& OnClosed() override
	{
		return OnHubConnectionClosedEvent;
	}

	/**
	 * Registers a handler for a hub method invocation.
	 *
	 * @param EventName The name of the hub method.
	 * @return Reference to the method invocation delegate.
	 */
	virtual FOnMethodInvocation& On(const FString& EventName) override;

	/**
	 * Invokes a hub method with the specified arguments and waits for the result.
	 *
	 * @param EventName The name of the hub method to invoke.
	 * @param InArguments The arguments to pass to the hub method.
	 * @return Reference to the method completion delegate that will be called when the method completes.
	 */
	virtual FOnMethodCompletion& Invoke(const FString& EventName, const TArray<FSignalRValue>& InArguments = TArray<FSignalRValue>()) override;

	/**
	 * Sends a hub method invocation with the specified arguments without waiting for a result.
	 *
	 * @param InEventName The name of the hub method to invoke.
	 * @param InArguments The arguments to pass to the hub method.
	 */
	virtual void Send(const FString& InEventName, const TArray<FSignalRValue>& InArguments = TArray<FSignalRValue>()) override;
#pragma endregion IHubConnection overrides

#pragma region FTickableGameObject overrides
public:
	/**
	 * Ticks the hub connection, used for periodic tasks such as sending pings.
	 *
	 * @param DeltaTime The time elapsed since the last tick.
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * Gets the stat ID for this tickable object.
	 *
	 * @return The stat ID.
	 */
	TStatId GetStatId() const override;

	/**
	 * Gets the tickable tick type for this object.
	 *
	 * @return The tickable tick type.
	 */
	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Always;
	}

	/**
	 * Determines if this object is tickable.
	 *
	 * @return True if tickable, false otherwise.
	 */
	virtual bool IsTickable() const override
	{
		return true;
	}

	/**
	 * Determines if this object is tickable in the editor.
	 *
	 * @return True if tickable in editor, false otherwise.
	 */
	virtual bool IsTickableInEditor() const override
	{
		return true;
	}

	/**
	 * Determines if this object is tickable when the game is paused.
	 *
	 * @return True if tickable when paused, false otherwise.
	 */
	virtual bool IsTickableWhenPaused() const override
	{
		return true;
	}
#pragma endregion FTickableGameObject overrides

protected:
	void ProcessMessage(const FString& InMessageStr);

private:
	enum class EConnectionState
	{
		Connecting,
		Connected,
		Disconnecting,
		Disconnected,
	};
	EConnectionState ConnectionState;

	void OnConnectionStarted();
	void OnConnectionFailed();
	void OnConnectionError(const FString& /* Error */);
	void OnConnectionClosed(int32 StatusCode, const FString& Reason, bool bWasClean);

	void TryReconnectIfNeeded();

	void Ping();
	void InvokeHubMethod(const FString& MethodName, const TArray<FSignalRValue>& InArguments, FName CallbackId);

	FString Host;

	TSharedPtr<IHubProtocol> HubProtocol;
	TSharedPtr<FConnection> Connection;
	TMap<FString, FOnMethodInvocation> InvocationHandlers;
	FCriticalSection InvocationHandlersGuard;
	FCallbackManager CallbackManager;

	bool bHandshakeReceived = false;

	float TickTimeCounter = 0;

	TArray<FString> WaitingCalls;

	FOnHubConnectedEvent OnHubConnectedEvent;
	FOnHubConnectionErrorEvent OnHubConnectionErrorEvent;
	FHubConnectionClosedEvent OnHubConnectionClosedEvent;

	void SendCloseMessage();

	bool bReceivedCloseMessage = false;
	bool bShouldReconnect = false;
};
