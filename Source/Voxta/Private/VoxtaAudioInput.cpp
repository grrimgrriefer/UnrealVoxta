// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAudioInput.h"
#include "RuntimeAudioImporter/ImportedSoundWave.h"

UVoxtaAudioInput::UVoxtaAudioInput()
{
	m_audioCaptureDevice = UCapturableSoundWave::CreateCapturableSoundWave();
}

void UVoxtaAudioInput::InitializeSocket(const FString& serverIP, int serverPort)
{
	if (m_connectionState != MicrophoneSocketState::NotConnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("Audio socket was already initialized, skipping new initialize attempt."));
		return;
	}
	m_connectionState = MicrophoneSocketState::Initializing;

	m_audioWebSocket = MakeShared<AudioWebSocket>(serverIP, serverPort);
	m_audioWebSocket->OnConnectedEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketConnected);
	m_audioWebSocket->OnConnectionErrorEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketConnectionError);
	m_audioWebSocket->OnClosedEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketClosed);
	m_audioWebSocket->Connect();
}

void UVoxtaAudioInput::OnSocketConnected()
{
	m_audioWebSocket->Send("{\"contentType\":\"audio/wav\",\"sampleRate\":48000,"
							"\"channels\":2,\"bitsPerSample\": 16,\"bufferMilliseconds\":30}");
	m_connectionState = MicrophoneSocketState::Ready;

	StartStreaming();
}

void UVoxtaAudioInput::OnSocketConnectionError(const FString& error)
{
	UE_LOG(LogTemp, Error, TEXT("Audio socket was closed due to error: %s"), *error);
	m_connectionState = MicrophoneSocketState::Closed;
}

void UVoxtaAudioInput::OnSocketClosed(int StatusCode, const FString& Reason, bool bWasClean)
{
	if (!bWasClean)
	{
		UE_LOG(LogTemp, Warning, TEXT("Audio socket was improperly closed because of reason: %s %d"), *Reason, StatusCode);
	}
	m_connectionState = MicrophoneSocketState::Closed;
}

void UVoxtaAudioInput::StartStreaming()
{
	if (m_connectionState != MicrophoneSocketState::Ready)
	{
		return;
	}

	m_audioCaptureDevice->OnHeyo.AddDynamic(this, &UVoxtaAudioInput::OnAudioDataAdded);
	UE_LOG(LogTemp, Warning, TEXT("Starting audio capture"));
	m_audioCaptureDevice->StartCapture(0);
	m_connectionState = MicrophoneSocketState::InUse;
}

UCapturableSoundWave* UVoxtaAudioInput::StopStreaming()
{
	m_audioCaptureDevice->StopCapture();
	m_audioCaptureDevice->OnHeyo.RemoveDynamic(this, &UVoxtaAudioInput::OnAudioDataAdded);
	m_connectionState = MicrophoneSocketState::Ready;
	return m_audioCaptureDevice;
}

void UVoxtaAudioInput::CloseSocket()
{
	m_audioWebSocket->Close();
}

void UVoxtaAudioInput::OnAudioDataAdded(const TArray<uint8>& PopulatedAudioData)
{
	m_audioWebSocket->Send(PopulatedAudioData.GetData(), PopulatedAudioData.Num());
}