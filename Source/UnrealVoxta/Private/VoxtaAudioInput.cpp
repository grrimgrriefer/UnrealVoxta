// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAudioInput.h"
#include "VoxtaClient.h"
#include "RuntimeAudioImporter/ImportedSoundWave.h"
#include "Logging/StructuredLog.h"
#include "AudioWebSocket.h"
#include "ChatSession.h"

void UVoxtaAudioInput::InitializeSocket(int bufferMs, int sampleRate, int inputChannels)
{
	if (m_connectionState != VoxtaMicrophoneState::Uninitialized)
	{
		UE_LOGFMT(VoxtaLog, Warning, "Audio socket was already initialized, skipping new initialize attempt.");
		return;
	}

	m_voxtaClient = Cast<UVoxtaClient>(GetOuter());
	VoxtaClientState clientCurrentState = m_voxtaClient->GetCurrentState();
	if (clientCurrentState == VoxtaClientState::Disconnected ||
		clientCurrentState == VoxtaClientState::AttemptingToConnect ||
		clientCurrentState == VoxtaClientState::Terminated)
	{
		UE_LOGFMT(VoxtaLog, Error, "Audio socket could not be initialized as the VoxtaClient is currently not ready.");
		return;
	}

	m_bufferMs = bufferMs;
	m_sampleRate = sampleRate;
	m_inputChannels = inputChannels;
	m_connectionState = VoxtaMicrophoneState::NotConnected;
	m_audioWebSocket = MakeShared<AudioWebSocket>(m_voxtaClient->GetServerAddress(),
		m_voxtaClient->GetServerPort());

	m_audioWebSocket->OnConnectedEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketConnected);
	m_audioWebSocket->OnConnectionErrorEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketConnectionError);
	m_audioWebSocket->OnClosedEvent.AddUObject(this, &UVoxtaAudioInput::OnSocketClosed);	
}

void UVoxtaAudioInput::DisconnectFromChat()
{
	UE_LOGFMT(VoxtaLog, Warning, "Disconnecting socket & shutting down voice capture gracefully.");

	m_audioCaptureDevice.ShutDown();
	if (m_audioWebSocket != nullptr)
	{
		m_audioWebSocket->Close();
	}
	m_connectionState = VoxtaMicrophoneState::Uninitialized;
}

void UVoxtaAudioInput::ConnectToCurrentChat()
{
	const FChatSession* chat = m_voxtaClient->GetChatSession();
	if (chat == nullptr)
	{
		UE_LOGFMT(VoxtaLog, Error, "Tried to connect Voice input to chat but there's no active chat, this should be impossible");
		return;
	}

	if (!chat->GetActiveServices().Contains(VoxtaServiceType::SpeechToText))
	{
		UE_LOGFMT(VoxtaLog, Error, "Tried to connect Voice input to chat but it doesn't support STT, this should be impossible");
		return;
	}
	else
	{
		m_connectionState = VoxtaMicrophoneState::Connecting;
		m_audioWebSocket->Connect(chat->GetSessionId());
	}
}

VoxtaMicrophoneState UVoxtaAudioInput::GetCurrentState() const
{
	return m_connectionState;
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

		UE_LOGFMT(VoxtaLog, Log, "Using predefined silence tresholds.");
		ConfigureSilenceTresholds(m_micNoiseGateThreshold, m_silenceDetectionThreshold, m_micInputGain);
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
	}
	else
	{
		UE_LOGFMT(VoxtaLog, Warning, "Attempted to stop streaming AudioInput via the socket, but it is not in use. ");
	}
	m_audioCaptureDevice.StopCapture();
	m_connectionState = VoxtaMicrophoneState::Ready;
}

bool UVoxtaAudioInput::IsRecording() const
{
	return m_connectionState == VoxtaMicrophoneState::InUse;
}

float UVoxtaAudioInput::GetInputTrueDecibels() const
{
	return m_audioCaptureDevice.GetTrueDecibels();
}

float UVoxtaAudioInput::GetRealtimeDecibels() const
{
	return m_audioCaptureDevice.GetRealtimeDecibels();
}

const FString& UVoxtaAudioInput::GetInputDeviceName() const
{
	return m_audioCaptureDevice.GetDeviceName();
}

void UVoxtaAudioInput::ConfigureSilenceTresholds(float micNoiseGateThreshold, float silenceDetectionThreshold, float micInputGain)
{
	m_micNoiseGateThreshold = micNoiseGateThreshold;
	m_silenceDetectionThreshold = silenceDetectionThreshold;
	m_micInputGain = micInputGain;
	m_audioCaptureDevice.ConfigureSilenceTresholds(m_micNoiseGateThreshold, m_silenceDetectionThreshold, m_micInputGain);
	return;
}

void UVoxtaAudioInput::ChatSessionHandshake()
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
		DisconnectFromChat();
	}
}

void UVoxtaAudioInput::OnSocketConnected()
{
	UE_LOGFMT(VoxtaLog, Log, "Succesfully connected Audiosocket (microphone input) to VoxtaServer.");

	ChatSessionHandshake();
}

void UVoxtaAudioInput::OnSocketConnectionError(const FString& error)
{
	UE_LOGFMT(VoxtaLog, Error, "AudioInput socket error: {0}. Closing socket.", error);
	DisconnectFromChat();
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
	m_connectionState = VoxtaMicrophoneState::NotConnected;
}