// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "WebSocketsModule.h"

class AUDIOUTILITY_API AudioWebSocket : public TSharedFromThis<AudioWebSocket>
{
public:
	AudioWebSocket(const FString& serverIP, int serverPort);

	bool Connect();
	void Close(int32 Code = 1000, const FString& Reason = FString());
	void Send(const char* buffer, unsigned int nBufferFrames);
	void Send(const FString& message);

private:
	IWebSocket::FWebSocketConnectedEvent OnConnectedEvent;
	IWebSocket::FWebSocketConnectionErrorEvent OnConnectionErrorEvent;
	IWebSocket::FWebSocketClosedEvent OnClosedEvent;

	TSharedPtr<IWebSocket> Connection;

	FString m_serverIP;
	int m_serverPort;
};
