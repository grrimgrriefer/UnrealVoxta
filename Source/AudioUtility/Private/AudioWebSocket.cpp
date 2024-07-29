// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AudioWebSocket.h"
#include "RuntimeAudioImporter/AudioStructs.h"
#include "Logging/StructuredLog.h"

AudioWebSocket::AudioWebSocket(const FString& serverIP, int serverPort) :
	m_serverIP(serverIP),
	m_serverPort(serverPort)
{
}

bool AudioWebSocket::Connect()
{
	// websocket url might change in future versions, this is still correct as of beta.v117
	const FString uri = FString::Format(*FString(TEXT("ws://{0}:{1}/ws/audio/input/stream")), { m_serverIP, m_serverPort });
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
		});
		m_socketConnection->OnConnectionError().AddLambda(
			[Self = TWeakPtr<AudioWebSocket>(AsShared())] (const FString& ErrString)
		{
			UE_LOGFMT(AudioLog, Warning, "Websocket err: {0}", ErrString);

			if (TSharedPtr<AudioWebSocket> SharedSelf = Self.Pin())
			{
				SharedSelf->OnConnectionErrorEvent.Broadcast(ErrString);
			}
		});
		m_socketConnection->OnClosed().AddLambda(
			[Self = TWeakPtr<AudioWebSocket>(AsShared())] (int32 StatusCode, const FString& Reason, bool bWasClean)
		{
			if (TSharedPtr<AudioWebSocket> SharedSelf = Self.Pin())
			{
				UE_LOGFMT(AudioLog, Log, "Websocket closing: Code={0}, Reason={1}, wasClean={2}",
					StatusCode,
					Reason,
					bWasClean);
				SharedSelf->OnClosedEvent.Broadcast(StatusCode, Reason, bWasClean);
			}
		});

		UE_LOGFMT(AudioLog, Log, "Connecting audio input websocket at: {0}", uri);
		m_socketConnection->Connect();
		return true;
	}
	else
	{
		UE_LOGFMT(AudioLog, Error, "Cannot start websocket.");
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
		UE_LOGFMT(AudioLog, Warning, "Cannot close the socket, audioWebSocket is already destroyed.");
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
		UE_LOGFMT(AudioLog, Error, "Cannot send audio data, audioWebSocket is destroyed.");
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
		UE_LOGFMT(AudioLog, Error, "Cannot send audio data, audioWebSocket is destroyed.");
	}
}