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
#include "IWebSocket.h"
#include "Interfaces/IHttpRequest.h"

/**
 * Represents a connection to a SignalR server.
 * Handles the underlying WebSocket connection and negotiation process with the server.
 */
class SIGNALR_API FConnection : public TSharedFromThis<FConnection>
{
public:
    /**
     * Creates a new connection to a SignalR server.
     *
     * @param InHost The URL of the SignalR server.
     * @param InHeaders HTTP headers to include in requests to the server.
     */
    FConnection(const FString& InHost, const TMap<FString, FString>& InHeaders);

    virtual ~FConnection();

    FConnection(const FConnection&) = delete;
    FConnection & operator=(const FConnection&) = delete;
    FConnection(FConnection&&) = delete;
    FConnection & operator=(FConnection&&) = delete;

    /**
     * Initiates the connection process by starting the negotiation with the server.
     */
    void Connect();

    /**
     * Checks if the connection is currently established.
     *
     * @return True if the connection is established, false otherwise.
     */
    bool IsConnected() const;

    /**
     * Sends data over the connection.
     *
     * @param Data The data to send.
     */
    void Send(const FString& Data);

    /**
     * Closes the connection.
     *
     * @param Code The status code for closing the connection. Default is 1000.
     * @param Reason The reason for closing the connection. Default is empty.
     */
    void Close(int32 Code = 1000, const FString& Reason = FString());

    DECLARE_EVENT(FConnection, FConnectionFailedEvent);

    /**
     * Gets the event that is triggered when the connection fails to establish.
     *
     * @return Reference to the connection failed event.
     */
    FConnectionFailedEvent& OnConnectionFailed();

    /**
     * Gets the event that is triggered when the connection is established.
     *
     * @return Reference to the connected event.
     */
    IWebSocket::FWebSocketConnectedEvent& OnConnected();

    /**
     * Gets the event that is triggered when a connection error occurs.
     *
     * @return Reference to the connection error event.
     */
    IWebSocket::FWebSocketConnectionErrorEvent& OnConnectionError();

    /**
     * Gets the event that is triggered when the connection is closed.
     *
     * @return Reference to the closed event.
     */
    IWebSocket::FWebSocketClosedEvent& OnClosed();

    /**
     * Gets the event that is triggered when a message is received.
     *
     * @return Reference to the message event.
     */
    IWebSocket::FWebSocketMessageEvent& OnMessage();

private:
    void Negotiate();
    void OnNegotiateResponse(FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bConnectedSuccessfully);
    void StartWebSocket();

    TSharedPtr<IWebSocket> Connection;
    FString Host;
    TMap<FString, FString> Headers;

    FConnectionFailedEvent OnConnectionFailedEvent;
    IWebSocket::FWebSocketConnectedEvent OnConnectedEvent;
    IWebSocket::FWebSocketConnectionErrorEvent OnConnectionErrorEvent;
    IWebSocket::FWebSocketClosedEvent OnClosedEvent;
    IWebSocket::FWebSocketMessageEvent OnMessageEvent;

    FString ConnectionToken;
    FString ConnectionId;

    static FString ConvertToWebsocketUrl(const FString& Url);
};
