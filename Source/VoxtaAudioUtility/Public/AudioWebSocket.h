// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "IWebSocket.h"

/**
 * AudioWebSocket
 * Main low level API to communicate with the audio streaming socket of VoxtaServer.
 * Internally relies on Unreal's FWebSocketsModule implementation.
 */
class VOXTAAUDIOUTILITY_API AudioWebSocket : public TSharedFromThis<AudioWebSocket>
{
#pragma region events
public:
	/** Called when a web socket connection has been established successfully. */
	IWebSocket::FWebSocketConnectedEvent OnConnectedEvent;

	/** Called when a web socket connection could not be established. */
	IWebSocket::FWebSocketConnectionErrorEvent OnConnectionErrorEvent;

	/** Called when a web socket connection has been closed. */
	IWebSocket::FWebSocketClosedEvent OnClosedEvent;
#pragma endregion

#pragma region public API
public:
	/**
	 * Register the address and port the socket should use to connect with VoxtaServer
	 *
	 * @param serverIP The VoxtaServer host IP address. (ipv4)
	 * @param serverPort The VoxtaServer host port.
	 */
	AudioWebSocket(const FString& serverIP, uint16 serverPort);

	/**
	 * Create the websocket through FWebSocketsModule and start up the connection.
	 * Call this after setting up event handlers or to reconnect after connection errors.
	 *
	 * @param sessionId The sessionId that we want to forward our audio to.
	 *
	 * @return False if we failed to create the websocket instance.
	 */
	bool Connect(const FGuid& sessionId);

	/**
	 * Manually close the websocket connection.
	 *
	 * @param code lws_close_status code of the libwebsockets for the closure. 1000 is normal.
	 * (https://libwebsockets.org/lws-api-doc-main/html/group__wsclose.html)
	 * @param reason Human readable string explaining why the connection is closing.
	 */
	void Close(int code = 1000, const FString& reason = FString());

	/**
	 * Send binary data through the websocket.
	 *
	 * @param buffer A pointer to the first byte of the data to be sent.
	 * @param nBufferFrames Number of bytes to send.
	 */
	void Send(const void* buffer, unsigned int nBufferFrames);

	/**
	 * Send string data through the websocket.
	 *
	 * @param message Data to be sent as a UTF-8 encoded string.
	 */
	void Send(const FString& message);
#pragma endregion

#pragma region data
private:
	TSharedPtr<IWebSocket> m_socketConnection;
	FString m_serverIP;
	uint16 m_serverPort;
	FGuid m_sessionId;
#pragma endregion
};
