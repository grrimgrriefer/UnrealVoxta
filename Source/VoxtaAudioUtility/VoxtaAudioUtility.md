# VoxtaAudioUtility Module

## Overview

The VoxtaAudioUtility module handles real-time audio capture, encoding, and decoding for the VoxtaUnreal plugin. It provides functionality for:

- Audio capture from microphone input
- WAV file format decoding
- Real-time audio streaming
- WebSocket-based audio transmission

## Core Components

### Audio Capture Handler

The `AudioCaptureHandler` class manages microphone input capture and provides audio data to the voice system. Key features include:

- Real-time audio capture from system microphones
- Audio buffer management and queueing
- Configurable capture settings (sample rate, channels, etc.)

### Audio WebSocket

`AudioWebSocket` handles WebSocket connections for transmitting audio data. Capabilities:

- Secure WebSocket connections (WSS)
- Binary audio data transmission
- Connection state management

### Voice Runner Thread

`VoiceRunnerThread` implements a dedicated thread for processing voice data. Features:

- Non-blocking audio processing
- Thread-safe data handling
- Event-based communication

## Runtime Audio Import

The module includes a runtime audio codec system for decoding various audio formats:

- WAV codec with dr_wav library integration
- Extensible codec framework for adding new formats
- Real-time decoding capabilities

### Key Classes

- `FBaseRuntimeCodec`: Base class for audio codecs
- `WAV_RuntimeCodec`: WAV format implementation
- `RuntimeAudioImporterLibrary`: Main interface for audio import operations

## Platform Support

The module supports:

- Android
- iOS
- Mac
- Windows

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
