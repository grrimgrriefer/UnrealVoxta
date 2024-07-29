// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "AudioWebSocket.h"
#include "AudioCaptureCore.h"
#include "Voice.h"
#include "RuntimeAudioImporter/AudioStructs.h"
#include "VoiceRunnerThread.h"

/// <summary>
/// Handles audio input by wrapping the engine's inner FVoiceModule.
/// On fixed intervals, a background thread will forward any captured audio bytes to the AudioWebSocket.
/// </summary>
class AUDIOUTILITY_API AudioCaptureHandler
{
public:
	/// <summary>
	/// Store a pointer to the websocket that will receive the parsed data from the FVoiceModule.
	/// </summary>
	/// <param name="socket">A shared pointer to the websocket used for sending the microphone data.</param>
	/// <param name="bufferMillisecondSize">The amount of milliseconds the buffer should account for.</param>
	void RegisterSocket(TSharedPtr<AudioWebSocket> socket, int bufferMillisecondSize);

	/// <summary>
	/// Tries to create a voice capture instance (IVoiceCapture).
	/// </summary>
	/// <param name="samplerate">The samplerate that will be requested from the microphone,
	/// this must match the specification sent thought the websocket.</param>
	/// <param name="numchannels">The amount of inputchannels that will be request from the microphone,
	/// this must match the specification sent thought the websocket.</param>
	/// <returns>Returns true if VoiceCapture was created successfully.</returns>
	bool TryInitialize(int32 samplerate = 16000, int32 numchannels = 1);

	/// <summary>
	/// Tries to start that IVoiceCapture and also start the background thread that will forward any
	/// captured data every fixed timestep.
	/// </summary>
	/// <returns>True if the voice capture is started successfully.</returns>
	bool TryStartCapture();

	/// <summary>
	/// Stop the voice capture, reset the buffers and stops the background thread.
	/// </summary>
	void StopCapture();

	/// <summary>
	/// Stops the capture and also clears the used memory of the background thread.
	/// </summary>
	void ShutDown();

	/// <summary>
	/// The current loudness of the microphone.
	/// Returns -1.0 if the capture is not supported or the capture is not initialized.
	/// </summary>
	/// <returns>Returns -1.0 if the capture is not supported or the capture is not initialized.</returns>
	float GetAmplitude() const;

	/// <summary>
	/// Returns the name of the device used by the VoiceModule, if initialized.
	/// </summary>
	/// <returns>Returns the name of the device used by the VoiceModule, if initialized.</returns>
	FString GetDeviceName() const;

	/// <summary>
	/// Used to gather the most recent audio data and send it to the websocket.
	/// </summary>
	void CaptureAndSendVoiceData();

private:
	friend class FVoiceRunnerThread;

	/// <summary>
	/// Buffer that contains the binary audio data captured between the fixed intervals of the thread.
	/// </summary>
	UPROPERTY()
	TArray<uint8> m_socketDataBuffer;

	/// <summary>
	/// The background thread that will invoke 'CaptureAndSendVoiceData' on fixed intervals.
	/// </summary>
	TUniquePtr<FVoiceRunnerThread> m_voiceRunnerThread;

	/// <summary>
	/// A pointer to the socket used to send the audioData through.
	/// </summary>
	TSharedPtr<AudioWebSocket> m_webSocket;

	/// <summary>
	/// A pointer to the voiceCapture that can be used to request the audio data from UE.
	/// </summary>
	TSharedPtr<class IVoiceCapture> m_voiceCaptureDevice;

	FString m_deviceName = TEXT("");
	bool m_isCapturing = false;
	int m_bufferMillisecondSize;

	void CaptureVoiceInternal();
	void SendInternal(const TArray<uint8> InData);
};
