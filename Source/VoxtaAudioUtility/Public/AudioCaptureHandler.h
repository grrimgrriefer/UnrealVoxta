// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "AudioWebSocket.h"
#include "AudioCaptureCore.h"
#include "Voice.h"
#include "RuntimeAudioImporter/AudioStructs.h"
#include "VoiceRunnerThread.h"
#include "Misc/ScopeLock.h"

/// <summary>
/// Handles audio input by wrapping the engine's inner FVoiceModule.
/// On fixed intervals, a background thread will forward any captured audio bytes to the AudioWebSocket.
/// </summary>
class VOXTAAUDIOUTILITY_API AudioCaptureHandler
{
public:
	/// <summary>
	/// Store a pointer to the websocket that will receive the parsed data from the FVoiceModule.
	/// </summary>
	/// <param name="socket">A shared pointer to the websocket used for sending the microphone data.</param>
	/// <param name="bufferMillisecondSize">The amount of milliseconds the buffer should account for.</param>
	void RegisterSocket(TWeakPtr<AudioWebSocket> socket, int bufferMillisecondSize);

	/// <summary>
	/// Tries to create a voice capture instance (IVoiceCapture).
	/// </summary>
	/// <param name="sampleRate">The sample rate that will be requested from the microphone,
	/// this must match the specification sent thought the websocket.</param>
	/// <param name="numChannels">The amount of input channels that will be request from the microphone,
	/// this must match the specification sent thought the websocket.</param>
	/// <returns>Returns true if VoiceCapture was created successfully.</returns>
	bool TryInitializeVoiceCapture(int sampleRate = 16000, int numChannels = 1);

	/// <summary>
	/// Configure values to help the microphone to pick up voice without background noise.
	/// </summary>
	/// <param name="micNoiseGateThreshold">The linear amplitude, anything below this will output silent audio data.</param>
	/// <param name="silenceDetectionThreshold">The linear amplitude, anything below this will not generate any audio data.</param>
	/// <param name="micInputGain">The linear amplitude muliplier applied to the input.</param>
	void ConfigureSilenceTresholds(float micNoiseGateThreshold = 0.001f,
		float silenceDetectionThreshold = 0.001f,
		float micInputGain = 6.0f);

	/// <summary>
	/// Tries to start that IVoiceCapture and also start the background thread that will forward any
	/// captured data every fixed timestep.
	/// </summary>
	/// <returns>True if the voice capture is started successfully.</returns>
	bool TryStartVoiceCapture();

	/// <summary>
	/// Stop the voice capture, reset the buffers and stops the background thread.
	/// </summary>
	void StopCapture();

	/// <summary>
	/// Stops the capture and also clears the used memory of the background thread.
	/// </summary>
	void ShutDown();

	float GetDecibels() const;

	/// <summary>
	/// Returns the name of the device used by the VoiceModule, if initialized.
	/// </summary>
	/// <returns>Returns the name of the device used by the VoiceModule, if initialized.</returns>
	const FString& GetDeviceName() const;

private:
	friend class FVoiceRunnerThread;

	const float DEFAULT_SILENCE_DECIBELS = -100.f;

	FString m_deviceName = TEXT("");
	bool m_isCapturing = false;
	int m_bufferMillisecondSize;
	float m_decibels = DEFAULT_SILENCE_DECIBELS;

	mutable FCriticalSection m_captureGuard;

	TUniquePtr<FVoiceRunnerThread> m_voiceRunnerThread;
	TWeakPtr<AudioWebSocket> m_webSocket;
	TSharedPtr<class IVoiceCapture> m_voiceCaptureDevice;
	/// <summary>
	/// Buffer that contains the binary audio data captured between the fixed intervals of the thread.
	/// </summary>
	UPROPERTY()
	TArray<uint8> m_socketDataBuffer;

	/// <summary>
	/// Used to gather the most recent audio data and send it to the websocket.
	/// </summary>
	void CaptureAndSendVoiceData();

	/// <summary>
	/// Populate the provided buffer with data captured from the VoiceCapture.
	/// </summary>
	/// <param name="rawData">The array that will be filled with the raw audio data.</param>
	void CaptureVoiceInternal(TArray<uint8>& voiceDataBuffer, float& decibels) const;

	/// <summary>
	/// Send the provided raw audio data to the socket.
	/// </summary>
	/// <param name="rawData">The array of raw audio data.</param>
	void SendInternal(const TArray<uint8> rawData) const;

	float AnalyseDecibels(const TArray<uint8>& VoiceData, uint32 DataSize) const;
};
