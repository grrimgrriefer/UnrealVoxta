// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Misc/ScopeLock.h"
#include "VoiceRunnerThread.h"
#include "VoxtaDefines.h"

class AudioWebSocket;
class IVoiceCapture;

/**
 * AudioCaptureHandler.
 * Handles audio input by wrapping the engine's inner FVoiceModule.
 * On fixed intervals, the FVoiceRunnerThread trigger this to forward any captured audio bytes to the AudioWebSocket.
 */
class VOXTAAUDIOUTILITY_API AudioCaptureHandler
{
#pragma region public API
public:
	/**
	 * Store a pointer to the websocket that will receive the parsed data from the FVoiceModule.
	 *
	 * @param socket A pointer to the websocket used for sending the microphone data.
	 * @param bufferMillisecondSize The amount of milliseconds the buffer should account for.
	 */
	void RegisterSocket(TWeakPtr<AudioWebSocket> socket, int bufferMillisecondSize);

	/**
	 * Tries to create a voice capture instance (IVoiceCapture).
	 *
	 * @param sampleRate The sample rate that will be requested from the microphone,
	 *  this must match the specification sent thought the websocket.
	 * @param numChannels The amount of input channels that will be request from the microphone,
	 *  this must match the specification sent thought the websocket.
	 *
	 * @return True if VoiceCapture was created successfully.
	 */
	bool TryInitializeVoiceCapture(int sampleRate = 16000, int numChannels = 1);

	/**
	 * Configure values to help the microphone to pick up voice without background noise.
	 *
	 * @param micNoiseGateThreshold The linear amplitude, anything below this will output silent audio data.
	 * @param silenceDetectionThreshold The linear amplitude, anything below this will not generate any audio data.
	 * @param micInputGain The linear amplitude muliplier applied to the input.
	 */
	void ConfigureSilenceTresholds(float micNoiseGateThreshold, float silenceDetectionThreshold, float micInputGain);

	/**
	 * Tries to start that IVoiceCapture and also start the background thread that will forward any
	 * captured data every fixed timestep.
	 *
	 * @return True if the voice capture is started successfully.
	 */
	bool TryStartVoiceCapture();

	/** Stop the voice capture, reset the buffers and stops the background thread. */
	void StopCapture();

	/** Stops the capture, permanently clearing the reserved memory for the background thread. */
	void ShutDown();

	/** @return The volume in decibels, of the last audioChunk (~30ms delay) */
	float GetTrueDecibels() const;

	/** @return The volume in decibels, more responsive but less accurate (amplitude based) */
	float GetRealtimeDecibels() const;

	/** @return Returns the name of the device used by the VoiceModule, if initialized. */
	const FString& GetDeviceName() const;
#pragma endregion

#pragma region data
private:
	UPROPERTY()
	TArray<uint8> m_socketDataBuffer;

	const float DEFAULT_SILENCE_DECIBELS = -100.f;

	/** The background thread needs access to our private functions. */
	friend class FVoiceRunnerThread;

	FString m_deviceName = EMPTY_STRING;
	bool m_isCapturing;
	int m_bufferMillisecondSize;
	float m_trueDecibels = DEFAULT_SILENCE_DECIBELS;

	mutable FCriticalSection m_captureGuard;
	TUniquePtr<FVoiceRunnerThread> m_voiceRunnerThread;
	TWeakPtr<AudioWebSocket> m_webSocket;
	TSharedPtr<IVoiceCapture> m_voiceCaptureDevice;
#pragma endregion

#pragma region private API
	/** Used to gather the most recent audio data and send it to the websocket. */
	void CaptureAndSendVoiceData();

	/**
	 * Populate the parameters with the values registered from the most recent inteval of the VoiceCapture.
	 *
	 * @param voiceDataBuffer The array of raw bytes that will be filled with the most recent inverval's data.
	 * @param decibels The volume in decibels that covers the most recent inverval's data.
	 */
	void CaptureVoiceInternal(TArray<uint8>& voiceDataBuffer, float& decibels) const;

	/**
	 * Send the provided raw audio data to VoxtaServer's microphone socket.
	 *
	 * @param rawData The array of bytes that will be sent.
	 */
	void SendInternal(const TArray<uint8> rawData) const;

	/**
	 * Analyze the provided data and calculate the maximum decibels that is present in this chunk of audio.
	 *
	 * @param VoiceData The raw bytes of audio data.
	 * @param DataSize The amount of bytes that should be part of the analysis
	 *
	 * @return The decibel value in float format.
	 */
	float AnalyseDecibels(const TArray<uint8>& VoiceData, uint32 DataSize) const;
#pragma endregion
};
