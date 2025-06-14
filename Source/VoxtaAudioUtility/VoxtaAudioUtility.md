# VoxtaAudioUtility Module

## Overview

The VoxtaAudioUtility module handles real-time audio capture, encoding- and decoding audio data from and to VoxtaServer.

- Audio capture from microphone input
- WAV data decoding
- Real-time audio streaming
- WebSocket-based audio transmission

## Module Structure

### Sending audio to VoxtaServer

- `AudioCaptureHandler`: Manages microphone input capture and provides audio data to the voice system.
  - Real-time audio capture from system microphones
  - Configurable capture settings (sample rate, channels, etc.)
  - Audio buffer management and queueing (triggered by `VoiceRunnerThread`)
- `AudioWebSocket`: Manages transmitting audio data through Unreal's FWebSocketsModule.
  - Connection state management
  - Broadcast notifications (connection, authenticaion, etc.)
  - Error handling
- `VoiceRunnerThread`: A dedicated non-blocking thread for processing voice data.
  - Every 0.2 seconds:
    - Fill a buffer with all available voice data
    - Calculate decibel level from the buffer
    - If the buffer contains data, send it to VoxtaServer 

![SequenceDiagramAudioUtility_send image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/SequenceDiagramAudioUtility_send.PNG&resolveLfs=true&%24format=octetStream "SequenceDiagramAudioUtility_send image.")  

### Receiving audio from VoxtaServer

- `FBaseRuntimeCodec`: Generic base codec to support more formats in the future.
- `WAV_RuntimeCodec`: WAV codec with dr_wav library integration
- `RuntimeAudioImporterLibrary`: Main interface for audio import operations

![SequenceDiagramAudioUtility_receive image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/SequenceDiagramAudioUtility_receive.PNG&resolveLfs=true&%24format=octetStream "SequenceDiagramAudioUtility_receive image.")  

## Platform Support

The module supports:

- Android*
- iOS*
- Mac*
- Windows

*untested

## Usage Examples

### Initializing Audio Capture

```cpp
TSharedPtr<AudioWebSocket> audioWebSocket = MakeShared<AudioWebSocket>(serverAddress, serverPort);

// Listen to connection-related broadcasts
audioWebSocket->OnConnectedEvent.AddUObject(this, &YourClass::OnSocketConnected);
audioWebSocket->OnConnectionErrorEvent.AddUObject(this, &YourClass::OnSocketConnectionError);
audioWebSocket->OnClosedEvent.AddUObject(this, &YourClass::OnSocketClosed);

// Create and initialize audio capture
AudioCaptureHandler audioCaptureDevice = AudioCaptureHandler();
audioCaptureDevice.ConfigureSilenceThresholds(0.001f, 0.001f, 6.0f); //  micNoiseGateThreshold, silenceDetectionThreshold, micInputGain

// Inform server of audio data format we will send
audioWebSocket->Send(FString::Format(TEXT("{\"contentType\":\"audio/wav\","
    "\"sampleRate\":{0},\"channels\":{1},\"bitsPerSample\": 16,\"bufferMilliseconds\":{2}}"),
    { sampleRate, inputChannels, bufferMs }));

audioCaptureDevice.RegisterSocket(audioWebSocket, 200); // audioWebSocket, bufferMs
audioCaptureDevice.TryInitializeVoiceCapture(16000, 1); // 16kHz, mono

// Connect to the current ongoing chat
audioWebSocket->Connect(chatSessionId);

// Start capturing
audioCaptureDevice->TryStartVoiceCapture();
```

### Decoding WAV data

```cpp
URuntimeAudioImporterLibrary::ImportAudioFromBuffer(TArray64<uint8>(rawAudioData),
    [Self = TWeakPtr<YourClass>(AsShared())] (UImportedSoundWave* soundWave)
    {
        if (soundWave)
        {
            if (TSharedPtr<MessageChunkAudioContainer> sharedSelf = Self.Pin())
            {
                // soundWave->AddToRoot();
                // ...
            }
        }
        else
        {
            UE_LOGFMT(VoxtaLog, Error, "Failed to process raw audio data into UImportedSoundWave.");
        }
    });
```

## Dependencies

- UnrealEngine
  - `AudioCaptureCore`
  - `AudioExtensions`
  - `AudioPlatformConfiguration`
  - `Core`
  - `Engine`
  - `Voice`
  - `WebSockets`
- UnrealVoxta
  - `VoxtaData`
  - `RuntimeAudioImporter_AndroidAPL`
- UnrealEngine (platform-specific)
  - `AVFoundation`
  - `AndroidPermission`
  - `AudioCaptureAndroid`
  - `AudioCaptureAudioUnit`
  - `AudioCaptureRtAudio`
  - `AudioToolbox`
  - `CoreAudio`

## Licensing

RuntimeAudioImporter:  
MIT license - copyright (c) 2024 Georgy Treshchev. See VoxtaAudioUtility/Public/RuntimeAudioImporter/LICENSE for details.

Other code:  
MIT license - copyright (c) 2025 grrimgrriefer & DZnnah. See LICENSE in root for details.
