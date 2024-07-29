// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "WebSocketsModule.h"

/// <summary>
/// Main low level API to communicate with the audio streaming IWebSocket.
/// Relies on Unreal's FWebSocketsModule implementation.
/// </summary>
class AUDIOUTILITY_API AudioWebSocket : public TSharedFromThis<AudioWebSocket>
{
public:
	/// <summary>
	/// Initialize the address that the socket should use to forward data to.
	/// </summary>
	/// <param name="serverIP">The host IP address. (ipv4)</param>
	/// <param name="serverPort">The host port.</param>
	AudioWebSocket(const FString& serverIP, int serverPort);

	/// <summary>
	/// Create the websocket through FWebSocketsModule and start up the connection.
	/// </summary>
	/// <returns>Returns false if we failed to create the websocket instance.</returns>
	bool Connect();

	/// <summary>
	/// Manually close the websocket connection.
	/// </summary>
	/// <param name="code">The lws_close_status code of the libwebsockets for the closure. 1000 is normal.</param>
	/// <param name="reason">Optional reason for the socket closing.</param>
	void Close(int code = 1000, const FString& reason = FString());

	/// <summary>
	/// Send binary data through the websocket.
	/// </summary>
	/// <param name="buffer">A pointer to the first element in the binary data array.</param>
	/// <param name="nBufferFrames">The amount of frames in the data array.</param>
	void Send(const void* buffer, unsigned int nBufferFrames);

	/// <summary>
	/// Send text data through the websocket.
	/// </summary>
	/// <param name="message">The text data to send.</param>
	void Send(const FString& message);

	/// <summary>
	/// Event fired when the WebSocket is connected.
	/// </summary>
	IWebSocket::FWebSocketConnectedEvent OnConnectedEvent;

	/// <summary>
	/// Event fired when the Websocket encountered an error.
	/// </summary>
	IWebSocket::FWebSocketConnectionErrorEvent OnConnectionErrorEvent;

	/// <summary>
	/// Event fired when the WebSocket is closed.
	/// </summary>
	IWebSocket::FWebSocketClosedEvent OnClosedEvent;

private:
	/// <summary>
	/// A shared ref to the socket instance managed by UE.
	/// </summary>
	TSharedPtr<IWebSocket> m_socketConnection;

	FString m_serverIP;
	int m_serverPort;
};
