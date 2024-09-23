// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AudioWebSocket.h"
#include "VoxtaDefines.h"
#include "Logging/StructuredLog.h"
#include "WebSocketsModule.h"

AudioWebSocket::AudioWebSocket(const FString& serverIP, uint16 serverPort) :
	m_serverIP(serverIP),
	m_serverPort(serverPort)
{
}

bool AudioWebSocket::Connect()
{
	// websocket url might change in future versions, this is still correct as of beta.v117
	const FString uri = FString::Format(*FString(TEXT("ws://{0}:{1}/ws/audio/input/stream")),
		{ m_serverIP, m_serverPort });
	m_socketConnection = FWebSocketsModule::Get().CreateWebSocket(uri, FString(), TMap<FString, FString>());

	if (m_socketConnection.IsValid())
	{
		m_socketConnection->OnConnected().AddLambda(
			[Self = TWeakPtr<AudioWebSocket>(AsShared())] ()
			{
				if (TSharedPtr<AudioWebSocket> SharedSelf = Self.Pin())
				{
					SharedSelf->OnConnectedEvent.Broadcast();
				}
				else
				{
					UE_LOGFMT(VoxtaLog, Warning, "AudioWebSocket connection was successful but the reference was "
						"no longer valid.");
				}
			});
		m_socketConnection->OnConnectionError().AddLambda(
			[Self = TWeakPtr<AudioWebSocket>(AsShared())] (const FString& errorString)
			{
				UE_LOGFMT(VoxtaLog, Warning, "Websocket err: {0}", errorString);

				if (TSharedPtr<AudioWebSocket> SharedSelf = Self.Pin())
				{
					SharedSelf->OnConnectionErrorEvent.Broadcast(errorString);
				}
				else
				{
					UE_LOGFMT(VoxtaLog, Warning, "AudioWebSocket received an error but the reference was "
						"no longer valid.");
				}
			});
		m_socketConnection->OnClosed().AddLambda(
			[Self = TWeakPtr<AudioWebSocket>(AsShared())] (int32 statusCode, const FString& reason, bool bWasClean)
			{
				if (TSharedPtr<AudioWebSocket> SharedSelf = Self.Pin())
				{
					UE_LOGFMT(VoxtaLog, Log, "Websocket closing: Code: {0}, reason: {1}, wasClean: {2}",
						statusCode, reason, bWasClean);
					SharedSelf->OnClosedEvent.Broadcast(statusCode, reason, bWasClean);
				}
				else
				{
					UE_LOGFMT(VoxtaLog, Warning, "AudioWebSocket closed itself but but the reference was "
						"no longer valid.");
				}
			});

		UE_LOGFMT(VoxtaLog, Log, "Connecting audio input websocket at: {0}", uri);
		m_socketConnection->Connect();
		return true;
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Cannot start websocket.");
		return false;
	}
}

void AudioWebSocket::Close(int code, const FString& reason)
{
	if (m_socketConnection.IsValid())
	{
		m_socketConnection->Close(code, reason);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "Cannot close the socket, audioWebSocket is already destroyed.");
	}
}

void AudioWebSocket::Send(const void* buffer, unsigned int nBufferFrames)
{
	if (m_socketConnection.IsValid())
	{
		m_socketConnection->Send(buffer, nBufferFrames, true);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Cannot send audio data, audioWebSocket is destroyed.");
	}
}

void AudioWebSocket::Send(const FString& message)
{
	if (m_socketConnection.IsValid())
	{
		m_socketConnection->Send(message);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Cannot send audio data, audioWebSocket is destroyed.");
	}
}