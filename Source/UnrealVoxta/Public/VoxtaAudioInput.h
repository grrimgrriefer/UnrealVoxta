// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "AudioUtility/Public/AudioCaptureHandler.h"
#include "VoxtaData/Public/VoxtaMicrophoneState.h"
#include "VoxtaAudioInput.generated.h"

class AudioWebSocket;

/**
 * UVoxtaAudioInput
 * Main public-facing class responsible for containing all AudioInput related logic.
 * Takes care of both the microphone input, as well as sending it over a websocket to the VoxtaServer.
 * Can be accessed via the voxta subsystem, voxtaClient->GetVoiceInputHandler()
 */
UCLASS(Category = "Voxta")
class UNREALVOXTA_API UVoxtaAudioInput : public UObject
{
	GENERATED_BODY()

public:
#pragma region delegate declarations
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVoxtaAudioInputInitialized);
	DECLARE_MULTICAST_DELEGATE(FVoxtaAudioInputInitializedNative);
#pragma endregion

#pragma region events
	/** Event fired after both the audiosocket and the capturedevice are initialized. */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaAudioInputInitialized VoxtaAudioInputInitializedEvent;
	FVoxtaAudioInputInitializedNative VoxtaAudioInputInitializedEventNative;
#pragma endregion

#pragma region public API
public:
	/**
	 * Creates the AudioWebSocket and connects audioSocket input hosted on the VoxtaServer's IP and port.
	 *
	 * Note: It will automatically connect the socket to the server initiate the JSON config handshake, so the server is ready to receive audio.
	 *
	 * @param bufferMs Microphone buffersize in Milliseconds. Bigger values is less taxing but introduces more delay.
	 * @param sampleRate The samplerate used for microphone, 16000 is preferred due to serverside conversion.
	 * @param inputChannels The input channels for the microphone, 1 is preferred.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void InitializeSocket(int bufferMs = 200, int sampleRate = 16000, int inputChannels = 1);

	/**
	 * Shuts down the Microphone audio stream and closes the websocket gracefully.
	 */
	void CloseSocket();

	/**
	 * Starts the voice capture, sending captured audiodata to the server in fixed timesteps (bufferMs).
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void StartStreaming();

	/**
	 * Stops the voice capture.
	 *
	 * Note: This will cancel the last batch of audio data. (i.e. 0-200ms data lost)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void StopStreaming();

	/** @return True if VoiceInput is actively being captured at this moment. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	bool IsRecording() const;

	/**
	 * @return The decibels of the last recorded audiodata iteration (updated every ~30ms)
	 *
	 * Note: These values are roughly in the range of -100db (silence) and 0db (max loudness)
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	float GetInputDecibels() const;

	/** @return The name reported by the hardware device. */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	FString GetInputDeviceName() const;
#pragma endregion

#pragma region data
private:
	int m_bufferMs;
	int m_sampleRate;
	int m_inputChannels;

	AudioCaptureHandler m_audioCaptureDevice;
	TSharedPtr<AudioWebSocket> m_audioWebSocket;
	VoxtaMicrophoneState m_connectionState;
#pragma endregion

#pragma region private API
private:
	/**
	 * Send the Socket header to the VoxtaServer, so it knows the wave format specs to expect.
	 * Followed by trying to initialize the VoiceCapture (request the samplerate & channels from the hardware)
	 * which is then connected to the socket.
	 *
	 * Note: This activates the mic, but it does NOT start capturing voice data. You must call StartStreaming to
	 * begin capturing the actual audiodata.
	 */
	void InitializeVoiceCapture();

#pragma region IWebSocket listeners
	void OnSocketConnected();
	void OnSocketConnectionError(const FString& error);
	void OnSocketClosed(int statusCode, const FString& reason, bool bWasClean);
#pragma endregion
#pragma endregion
};
