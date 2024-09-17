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
	VoxtaClientState clientCurrentState = voxtaClientReference->GetCurrentState();
	if (clientCurrentState == VoxtaClientState::Disconnected ||
		clientCurrentState == VoxtaClientState::AttemptingToConnect ||
		clientCurrentState == VoxtaClientState::Terminated)
	{
		UE_LOGFMT(VoxtaLog, Error, "Audio socket could not be initialized as the VoxtaClient is currently {0}.", );
		return;
	}

	m_bufferMs = bufferMs;
	m_sampleRate = sampleRate;
	m_inputChannels = inputChannels;
	m_connectionState = VoxtaMicrophoneState::Initializing;
	m_audioWebSocket = MakeShared<AudioWebSocket>(voxtaClientReference->GetServerAddress(),
		voxtaClientReference->GetServerPort());

	m_audioWebSocket->OnConnectedEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketConnected);
	m_audioWebSocket->OnConnectionErrorEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketConnectionError);
	m_audioWebSocket->OnClosedEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketClosed);
	m_audioWebSocket->Connect();
}

void UVoxtaAudioInput::CloseSocket()
{
	UE_LOGFMT(VoxtaLog, Warning, "Closing socket & shutting down voice capture gracefully.");

	m_audioCaptureDevice.ShutDown();
	if (m_audioWebSocket != nullptr)
	{
		m_audioWebSocket->Close();
	}
	m_connectionState = VoxtaMicrophoneState::Closed;
}

void UVoxtaAudioInput::StartStreaming()
{
	if (m_connectionState != VoxtaMicrophoneState::Ready)
	{
		UE_LOGFMT(VoxtaLog, Warning, "Attempting to start streaming AudioInput to VoxtaServer, but the socket was not "
			" ready, aborting attempt.");
		return;
	}

	if (m_audioCaptureDevice.TryStartVoiceCapture())
	{
		UE_LOGFMT(VoxtaLog, Log, "Started voice capture via AudioInput.");
		m_connectionState = VoxtaMicrophoneState::InUse;
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "Failed to start the audio capture. Socket will not stream data :(");
	}
}

void UVoxtaAudioInput::StopStreaming()
{
	if (m_connectionState == VoxtaMicrophoneState::InUse)
	{
		UE_LOGFMT(VoxtaLog, Log, "Stopping voice capture via AudioInput.");
		return;
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "Attempted to stop streaming AudioInput via the socket, but it is not in use. "
			"Current state: {0}");
	}
	m_audioCaptureDevice.StopCapture();
	m_connectionState = VoxtaMicrophoneState::Ready;
}

bool UVoxtaAudioInput::IsRecording() const
{
	return m_connectionState == VoxtaMicrophoneState::InUse;
}

float UVoxtaAudioInput::GetInputDecibels() const
{
	return m_audioCaptureDevice.GetDecibels();
}

const FString& UVoxtaAudioInput::GetInputDeviceName() const
{
	return m_audioCaptureDevice.GetDeviceName();
}

void UVoxtaAudioInput::InitializeVoiceCapture()
{
	const FString socketInitialHeader = FString::Format(*FString(TEXT("{\"contentType\":\"audio/wav\","
		"\"sampleRate\":{0},\"channels\":{1},\"bitsPerSample\": 16,\"bufferMilliseconds\":{2}}")),
		{ m_sampleRate, m_inputChannels, m_bufferMs });

	UE_LOGFMT(VoxtaLog, Log, "Sending AudioInput (microphone) format data to VoxtaServer: {0}", socketInitialHeader);
	m_audioWebSocket->Send(socketInitialHeader);

	m_audioCaptureDevice.RegisterSocket(m_audioWebSocket, m_bufferMs);

	UE_LOGFMT(VoxtaLog, Log, "Registered AudioSocket to the VoiceCapture device, attempting to initialize it...");
	if (m_audioCaptureDevice.TryInitializeVoiceCapture(m_sampleRate, m_inputChannels))
	{
		UE_LOGFMT(VoxtaLog, Log, "VoiceCapture ready & hooked up to the VoxtaServer audio socket.");

		m_connectionState = VoxtaMicrophoneState::Ready;
		VoxtaAudioInputInitializedEventNative.Broadcast();
		VoxtaAudioInputInitializedEvent.Broadcast();
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Error, "VoiceCapture failed to initialize. Closing websocket.");
		CloseSocket();
	}
}

void UVoxtaAudioInput::OnSocketConnected()
{
	UE_LOGFMT(VoxtaLog, Log, "Succesfully connected Audiosocket (microphone input) to VoxtaServer.", );

	InitializeVoiceCapture();
}

void UVoxtaAudioInput::OnSocketConnectionError(const FString& error)
{
	UE_LOGFMT(VoxtaLog, Error, "AudioInput socket error: {0}. Closing socket.", error);
	CloseSocket();
}

void UVoxtaAudioInput::OnSocketClosed(int statusCode, const FString& reason, bool wasClean)
{
	if (wasClean)
	{
		UE_LOGFMT(VoxtaLog, Log, "AudioInput socket was closed. Reason: {0} Code: {1}", reason, statusCode);
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "Audio socket was improperly closed because of reason: {0} Code: {1}",
			reason, statusCode);
	}
	m_connectionState = VoxtaMicrophoneState::Closed;
}