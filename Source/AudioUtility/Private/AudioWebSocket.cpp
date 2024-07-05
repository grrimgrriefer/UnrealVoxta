// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AudioWebSocket.h"
#include "RuntimeAudioImporter/AudioStructs.h"

AudioWebSocket::AudioWebSocket(const FString& serverIP, int serverPort) :
	m_serverIP(serverIP),
	m_serverPort(serverPort)
{
}

bool AudioWebSocket::Connect()
{
	const FString uri = FString::Format(*FString(TEXT("ws://{0}:{1}/ws/audio/input/stream")), { m_serverIP, m_serverPort });

	Connection = FWebSocketsModule::Get().CreateWebSocket(uri, FString(), TMap<FString, FString>());

	if (Connection.IsValid())
	{
		Connection->OnConnected().AddLambda([Self = TWeakPtr<AudioWebSocket>(AsShared())] ()
		{
			if (TSharedPtr<AudioWebSocket> SharedSelf = Self.Pin())
			{
				SharedSelf->OnConnectedEvent.Broadcast();
			}
		});
		Connection->OnConnectionError().AddLambda([Self = TWeakPtr<AudioWebSocket>(AsShared())] (const FString& ErrString)
		{
			UE_LOG(AudioLog, Warning, TEXT("Websocket err: %s"), *ErrString);

			if (TSharedPtr<AudioWebSocket> SharedSelf = Self.Pin())
			{
				SharedSelf->OnConnectionErrorEvent.Broadcast(ErrString);
			}
		});
		Connection->OnClosed().AddLambda([Self = TWeakPtr<AudioWebSocket>(AsShared())] (int32 StatusCode, const FString& Reason, bool bWasClean)
		{
			if (TSharedPtr<AudioWebSocket> SharedSelf = Self.Pin())
			{
				SharedSelf->OnClosedEvent.Broadcast(StatusCode, Reason, bWasClean);
			}
		});

		UE_LOG(AudioLog, Log, TEXT("Connecting audio input websocket."));
		Connection->Connect();
		return true;
	}
	else
	{
		UE_LOG(AudioLog, Error, TEXT("Cannot start websocket."));
		return false;
	}
}

void AudioWebSocket::Close(int32 Code, const FString& Reason)
{
	if (Connection.IsValid())
	{
		Connection->Close(Code, Reason);
	}
	else
	{
		UE_LOG(AudioLog, Error, TEXT("Cannot close non connected websocket."));
	}
}

void AudioWebSocket::Send(const char* buffer, unsigned int nBufferFrames)
{
	if (Connection.IsValid())
	{
		Connection->Send(buffer, nBufferFrames, true);
	}
	else
	{
		UE_LOG(AudioLog, Error, TEXT("Cannot send data to non connected websocket."));
	}
}

void AudioWebSocket::Send(const FString& message)
{
	if (Connection.IsValid())
	{
		UE_LOG(AudioLog, Log, TEXT("Sending: %s"), *message);
		Connection->Send(message);
	}
	else
	{
		UE_LOG(AudioLog, Error, TEXT("Cannot send data to non connected websocket."));
	}
}