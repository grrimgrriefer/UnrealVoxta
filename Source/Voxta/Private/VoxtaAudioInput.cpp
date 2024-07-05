// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaAudioInput.h"

UVoxtaAudioInput::UVoxtaAudioInput()
{
}

void UVoxtaAudioInput::RegisterEndpoint(const FString& serverIP, int serverPort)
{
	m_audioWebSocket = MakeShared<AudioWebSocket>(serverIP, serverPort);
	m_audioWebSocket->Connect();
}

void UVoxtaAudioInput::StartStreaming()
{
	if (m_isStartingUp || m_isStreaming)
	{
		return;
	}

	if (false)// !m_audioCaptureDevice.IsInitialized())
	{
		m_isStartingUp = true;
		/*		m_startupThread = std::jthread([this] ()
					{
						if (m_initializedStatusOutput)
						{
							m_initializedStatusOutput("Initializing audio socket");
						}
						m_audioWebSocket->Connect();
						std::this_thread::sleep_for(std::chrono::milliseconds(500));

						m_audioWebSocket->Send("{\"contentType\":\"audio/wav\",\"sampleRate\":16000,"
							"\"channels\":1,\"bitsPerSample\": 16,\"bufferMilliseconds\":30}");
						std::this_thread::sleep_for(std::chrono::milliseconds(500));

						m_audioCaptureDevice.RegisterSocket(m_audioWebSocket);
						m_audioCaptureDevice.TryInitialize();
						if (m_initializedStatusOutput)
						{
							m_initializedStatusOutput(std::format("Warming up input device {}",
								m_audioCaptureDevice.GetDeviceName()));
						}

						std::this_thread::sleep_for(std::chrono::milliseconds(500));

						m_audioCaptureDevice.StartStream();
						if (m_initializedStatusOutput)
						{
							m_initializedStatusOutput("Microphone connected.");
						}

						m_isStreaming = true;
						m_isStartingUp = false;
					});
					*/
	}
	else
	{
		//		m_audioCaptureDevice.StartStream();
		m_isStreaming = true;
	}
}

void UVoxtaAudioInput::StopStreaming()
{
	if (m_isStartingUp || !m_isStreaming)
	{
		return;
	}

	//	m_audioCaptureDevice.StopStream();
	m_isStreaming = false;
}