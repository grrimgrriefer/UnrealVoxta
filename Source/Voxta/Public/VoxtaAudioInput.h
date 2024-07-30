// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioUtility/Public/AudioWebSocket.h"
#include "AudioUtility/Public/AudioCaptureHandler.h"
#include "VoxtaData/Public/VoxtaMicrophoneState.h"
#include "VoxtaAudioInput.generated.h"

/// <summary>
/// Main public-facing class responsible for containing all AudioInput related logic.
/// Takes care of both the microphone input, as well as sending it over a websocket to the VoxtaServer.
/// </summary>
UCLASS(HideCategories = (Mobility, Rendering, LOD), Blueprintable, ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class VOXTA_API UVoxtaAudioInput : public UActorComponent
{
	GENERATED_BODY()

public:
	/// <summary>
	/// Creates the AudioWebSocket with the provided IP and port.
	/// Note: It will automatically connect the socket to the server initiate the JSON config handshake, so the server is ready to receive audio.
	/// </summary>
	/// <param name="serverIP">The ipv4 address where the VoxtaServer is hosted.</param>
	/// <param name="serverPort">The port used to reach the VoxtaServer (should always be 5384, I think?)</param>
	/// <param name="bufferMs">Microphone buffersize in Milliseconds. Bigger values is less taxing but introduces more delay.</param>
	/// <param name="sampleRate">The samplerate used for microphone, 16000 is ideal due to serverside conversion.</param>
	/// <param name="inputChannels">The input channels for the microphone, 1 is ideal.</param>
	void InitializeSocket(const FString& serverIP,
		int serverPort,
		int bufferMs = 200,
		int sampleRate = 16000,
		int inputChannels = 1);

	/// <summary>
	/// Shuts down the Microphone audio stream and closes the websocket gracefully.
	/// </summary>
	void CloseSocket();

	/// <summary>
	/// Starts the voice capture, sending captured audiodata to the server in fixed timesteps (bufferMs).
	/// </summary>
	void StartStreaming();

	/// <summary>
	/// Stops the voice capture.
	/// Note: This will cancel the last batch of audio data. (i.e. 0-200ms data lost)
	/// </summary>
	void StopStreaming();

	/// <summary>
	/// Returns true if the AudioCapture device is actively streaming data to the VoxtaServer.
	/// </summary>
	/// <returns>Returns true if the AudioCapture device is actively streaming data.</returns>
	bool IsRecording() const;

	// TODO: spaw this with decibels, amplitude is kinda useless.
	float GetNormalizedAmplitude() const;

	/// <summary>
	/// Returns the name reported by the hardware device.
	/// The formatting has no fixed standards.
	/// </summary>
	/// <returns></returns>
	FString GetInputDeviceName() const;

	///~ Begin UActorComponent overrides.
protected:
	virtual void BeginDestroy() override;
	///~ End UActorComponent overrides.

private:
	int m_bufferMs;
	int m_sampleRate;
	int m_inputChannels;

	AudioCaptureHandler m_audioCaptureDevice;
	TSharedPtr<AudioWebSocket> m_audioWebSocket;
	VoxtaMicrophoneState m_connectionState;

	///~ Begin event-listeners for the IWebSocket interface
private:
	void OnSocketConnected();
	void OnSocketConnectionError(const FString& error);
	void OnSocketClosed(int statusCode, const FString& reason, bool bWasClean);
	//~ End event-listeners for the IWebSocket interface
};
