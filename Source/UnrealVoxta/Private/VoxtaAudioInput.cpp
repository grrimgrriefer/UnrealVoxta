// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAudioInput.h"
#include "VoxtaClient.h"
#include "RuntimeAudioImporter/ImportedSoundWave.h"
#include "Logging/StructuredLog.h"

void UVoxtaAudioInput::InitializeSocket(int bufferMs, int sampleRate, int inputChannels)
{
	if (m_connectionState != VoxtaMicrophoneState::NotConnected)
	{
		UE_LOGFMT(VoxtaLog, Warning, "Audio socket was already initialized, skipping new initialize attempt.");
		return;
	}

	UVoxtaClient* voxtaClientReference = Cast<UVoxtaClient>(GetOuter());
	VoxtaClientState mainClientState = voxtaClientReference->GetCurrentState();
	if (mainClientState == VoxtaClientState::Disconnected ||
		mainClientState == VoxtaClientState::AttemptingToConnect ||
		mainClientState == VoxtaClientState::Terminated)
	{
		UE_LOGFMT(VoxtaLog, Error, "Audio socket could not be initialized as the VoxtaClient is currently {0}.", );
		return;
	}

	m_connectionState = VoxtaMicrophoneState::Initializing;

	m_bufferMs = bufferMs;
	m_sampleRate = sampleRate;
	m_inputChannels = inputChannels;

	m_audioWebSocket = MakeShared<AudioWebSocket>(voxtaClientReference->GetServerAddress(), voxtaClientReference->GetServerPort());
	m_audioWebSocket->OnConnectedEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketConnected);
	m_audioWebSocket->OnConnectionErrorEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketConnectionError);
	m_audioWebSocket->OnClosedEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketClosed);
	m_audioWebSocket->Connect();
}

void UVoxtaAudioInput::OnSocketConnected()
{
	const FString socketInitialHeader = FString::Format(*FString(TEXT("{\"contentType\":\"audio/wav\",\"sampleRate\":{0},"
		"\"channels\":{1},\"bitsPerSample\": 16,\"bufferMilliseconds\":{2}}")), { m_sampleRate, m_inputChannels, m_bufferMs });

	UE_LOGFMT(VoxtaLog, Log, "Audio input socket config: {0}", socketInitialHeader);

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
	UE_LOGFMT(VoxtaLog, Error, "Audio socket was closed due to error: {0}", error);
	m_connectionState = VoxtaMicrophoneState::Closed;
}

void UVoxtaAudioInput::OnSocketClosed(int statusCode, const FString& reason, bool bWasClean)
{
	if (!bWasClean)
	{
		UE_LOGFMT(VoxtaLog, Warning, "Audio socket was improperly closed because of reason: {0} Code: {1}", reason, statusCode);
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
		UE_LOGFMT(VoxtaLog, Log, "Starting audio capture");
		m_connectionState = VoxtaMicrophoneState::InUse;
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Failed to start the audio capture");
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