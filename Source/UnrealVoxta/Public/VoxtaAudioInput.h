// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaData/Public/VoxtaMicrophoneState.h"
#include "VoxtaAudioUtility/Public/AudioCaptureHandler.h"
#include "AudioWebSocket.h"  
#include "VoxtaAudioInput.generated.h"

class UVoxtaClient;

/**
 * UVoxtaAudioInput
 * Main public-facing class responsible for containing all AudioInput related logic.
 * Takes care of both the microphone input, as well as sending it over a websocket to the VoxtaServer.
 *
 * Note: You should not instantiate this manually, just access it via the voxta subsystem:
 * [ GetWorld()->GetGameInstance()->GetSubsystem<UVoxtaClient>() ]->GetVoiceInputHandler() ]
 */
UCLASS(Category = "Voxta")
class UNREALVOXTA_API UVoxtaAudioInput : public UObject
{
	GENERATED_BODY()

#pragma region delegate declarations
public:
	/** Delegate fired when the audio input state changes. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxtaAudioInputStateChanged, VoxtaMicrophoneState, newState);
	/** Native C++ delegate for audio input state changes. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FVoxtaAudioInputStateChangedNative, VoxtaMicrophoneState);
#pragma endregion

#pragma region events
public:
	/** Event fired after both the audiosocket and the capturedevice are initialized. */
	UPROPERTY(BlueprintAssignable, Category = "Voxta", meta = (IsBindableEvent = "True"))
	FVoxtaAudioInputStateChanged VoxtaAudioInputStateChangedEvent;
	FVoxtaAudioInputStateChangedNative VoxtaAudioInputStateChangedEventNative;
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
	void InitializeSocket(int bufferMs = 200, int sampleRate = 16000, int inputChannels = 1);

	/**
	 * Used for cleaning up resources, should only be triggered when shutting down the game.
	 */
	void Cleanup();

	/** @return True if the audio input is initialized. */
	bool IsInitialized() const;

	/** Connects the audio input to the current chat session. */
	void ConnectToCurrentChat();

	/** Disconnects the audio input from the current chat session. */
	void DisconnectFromChat();

	/**
	 * Starts audio testing mode with the given sample rate and input channels.
	 * @param sampleRate The sample rate to use.
	 * @param inputChannels The number of input channels.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void StartAudioTesting(int sampleRate = 16000, int inputChannels = 1);

	/** Stops audio testing mode. */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void StopAudioTesting();

	/** Starts streaming audio data to the server. */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void StartStreaming(bool isTestMode = false);

	/** Stops streaming audio data to the server. */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void StopStreaming();

	/** @return The current microphone state. */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	VoxtaMicrophoneState GetCurrentState() const;

	/** @return True if VoiceInput is actively being captured at this moment. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	bool IsRecording() const;

	/** @return True if the input is currently silent. */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	bool IsInputSilent() const;

	/**
	 * @return The decibels of the last recorded audiodata iteration (updated every ~30ms)
	 *
	 * Note: These values are roughly in the range of -100db (silence) and 0db (max loudness)
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	float GetInputDecibels() const;

	/** @return The an immutable reference to the name reported by the hardware device. */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	const FString& GetInputDeviceName() const;

	/**
	 * Configure values to help the microphone to pick up voice without background noise.
	 *
	 * @param micNoiseGateThreshold The linear amplitude, anything below this will output silent audio data.
	 * @param silenceDetectionThreshold The linear amplitude, anything below this will not generate any audio data.
	 * @param micInputGain The linear amplitude muliplier applied to the input.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	void ConfigureSilenceThresholds(float micNoiseGateThreshold, float silenceDetectionThreshold, float micInputGain);
#pragma endregion

#pragma region UObject overrides
	/** Fallback in case the Cleanup was not called, should not be necessary though, but oh well. */
	virtual void BeginDestroy() override;
#pragma endregion

#pragma region data
private:
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Current Mic Noise Gate Threshold"))
	float m_micNoiseGateThreshold = 0.001f;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Current Silence Detection Threshold"))
	float m_silenceDetectionThreshold = 0.001f;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Current Mic Input Gain"))
	float m_micInputGain = 6.f;

	int m_bufferMs = 0;
	int m_sampleRate = 0;
	int m_inputChannels = 0;

	UPROPERTY()
	UVoxtaClient* m_voxtaClient = nullptr;

	AudioCaptureHandler m_audioCaptureDevice;
	TSharedPtr<AudioWebSocket> m_audioWebSocket = nullptr;
	VoxtaMicrophoneState m_connectionState = VoxtaMicrophoneState::Uninitialized;

	FDelegateHandle m_connectedHandle;
	FDelegateHandle m_connectionErrorHandle;
	FDelegateHandle m_closedHandle;
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
	void ChatSessionHandshake();

	void UpdateConnectionState(VoxtaMicrophoneState newState);

#pragma region IWebSocket listeners
private:
	/** Triggered by the SignalR socket when connected (success). Not guarenteed to be on GameThread. */
	void OnSocketConnected();
	/** Triggered by the SignalR socket when could not connect (failure). Not guarenteed to be on GameThread. */
	void OnSocketConnectionError(const FString& error);
	/** Triggered by the SignalR socket when was closed (intentional / forced). Not guarenteed to be on GameThread. */
	void OnSocketClosed(int statusCode, const FString& reason, bool wasClean);
#pragma endregion
#pragma endregion
};
