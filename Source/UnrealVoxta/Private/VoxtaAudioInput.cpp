// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAudioInput.h"
#include "RuntimeAudioImporter/ImportedSoundWave.h"

void UVoxtaAudioInput::InitializeSocket(const FString& serverIP,
	int serverPort,
	int bufferMs,
	int sampleRate,
	int inputChannels)
{
	if (m_connectionState != VoxtaMicrophoneState::NotConnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("Audio socket was already initialized, skipping new initialize attempt."));
		return;
	}
	m_connectionState = VoxtaMicrophoneState::Initializing;

	m_bufferMs = bufferMs;
	m_sampleRate = sampleRate;
	m_inputChannels = inputChannels;

	m_audioWebSocket = MakeShared<AudioWebSocket>(serverIP, serverPort);
	m_audioWebSocket->OnConnectedEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketConnected);
	m_audioWebSocket->OnConnectionErrorEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketConnectionError);
	m_audioWebSocket->OnClosedEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketClosed);
	m_audioWebSocket->Connect();
}

void UVoxtaAudioInput::OnSocketConnected()
{
	const FString socketInitialHeader = FString::Format(*FString(TEXT("{\"contentType\":\"audio/wav\",\"sampleRate\":{0},"
		"\"channels\":{1},\"bitsPerSample\": 16,\"bufferMilliseconds\":{2}}")), { m_sampleRate, m_inputChannels, m_bufferMs });

	UE_LOG(LogTemp, Log, TEXT("Audio input socket config: %s"), *socketInitialHeader);

	m_audioWebSocket->Send(socketInitialHeader);

	m_audioCaptureDevice.RegisterSocket(m_audioWebSocket, m_bufferMs);
	if (m_audioCaptureDevice.TryInitializeVoiceCapture(m_sampleRate, m_inputChannels))
	{
		m_connectionState = VoxtaMicrophoneState::Ready;
		VoxtaAudioInputInitializedEvent.Broadcast();
	}
}

void UVoxtaAudioInput::OnSocketConnectionError(const FString& error)
{
	UE_LOG(LogTemp, Error, TEXT("Audio socket was closed due to error: %s"), *error);
	m_connectionState = VoxtaMicrophoneState::Closed;
}

void UVoxtaAudioInput::OnSocketClosed(int statusCode, const FString& reason, bool bWasClean)
{
	if (!bWasClean)
	{
		UE_LOG(LogTemp, Warning, TEXT("Audio socket was improperly closed because of reason: %s %d"), *reason, statusCode);
	}
	m_connectionState = VoxtaMicrophoneState::Closed;
}

void UVoxtaAudioInput::StartStreaming()
{
	if (m_connectionState != VoxtaMicrophoneState::Ready)
	{
		return;
	}

	if (m_audioCaptureDevice.TryStartVoiceCapture())
	{
		UE_LOG(LogTemp, Log, TEXT("Starting audio capture"));
		m_connectionState = VoxtaMicrophoneState::InUse;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to start the audio capture"));
	}
}

void UVoxtaAudioInput::StopStreaming()
{
	if (m_connectionState != VoxtaMicrophoneState::InUse)
	{
		return;
	}
	m_audioCaptureDevice.StopCapture();
	m_connectionState = VoxtaMicrophoneState::Ready;
}

void UVoxtaAudioInput::CloseSocket()
{
	m_audioCaptureDevice.ShutDown();
	if (m_audioWebSocket != nullptr)
	{
		m_audioWebSocket->Close();
	}
	m_connectionState = VoxtaMicrophoneState::Closed;
}

bool UVoxtaAudioInput::IsRecording() const
{
	return m_connectionState == VoxtaMicrophoneState::InUse;
}

float UVoxtaAudioInput::GetNormalizedAmplitude() const
{
	return m_audioCaptureDevice.GetAmplitude() / 20.f; //TODO: swap amplitude for decibels
}

FString UVoxtaAudioInput::GetInputDeviceName() const
{
	return m_audioCaptureDevice.GetDeviceName();
}