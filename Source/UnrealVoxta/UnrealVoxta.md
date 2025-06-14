# UnrealVoxta Module

This module provides the core Voxta integration for Unreal Engine, handling state tracking, audio playback, and communication with the Voxta server.

## Main components

### UVoxtaClient
The main public-facing subsystem for Voxta integration. Manages:
- Stateful connection to VoxtaServer
- Chat session lifecycle 
- Audio input/output
- Character state management
- Event broadcasting

Access via:
```cpp
UVoxtaClient* Client = GetWorld()->GetGameInstance()->GetSubsystem<UVoxtaClient>();
```

### UVoxtaAudioPlayback 
Component that handles audio playback and lipsync for AI character responses. Features:
- Character-specific audio playback
- Multiple lipsync types (OVRLipSync, Audio2Face, Custom)
- Automatic audio download and processing
- Sequence management for multi-chunk responses

### UVoxtaAudioInput
Handles microphone input and streaming to the Voxta server:
- Configurable input buffer and format settings
- Automatic reconnection handling
- Voice activity detection
- Silent mode toggles

## Sequence diagram

Example flow of authentication, starting chat, receiving a response and sending microphone data. Along with fetching the character thumbnail.  
Depending on your use case, the sequence will differ ofc.

![SequenceDiagramUnrealVoxta image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/SequenceDiagramUnrealVoxta.PNG&resolveLfs=true&%24format=octetStream "SequenceDiagramUnrealVoxta image.")

## Getting Started (C++)

1. Add the UnrealVoxta plugin to your project
2. Add UVoxtaClient to your GameInstance subsystems
3. Initialize connection to Voxta server:
```cpp
UVoxtaClient* m_voxtaClient = GetWorld()->GetGameInstance()->GetSubsystem<UVoxtaClient>();
m_voxtaClient->StartConnection(FString(TEXT("127.0.0.1")), 5384);
```

4. Listen for connection and character events:
```cpp
// Character list available after connection
Client->VoxtaClientStateChangedEvent.AddDynamic(this, &ThisClass::OnVoxtaStateChanged);
Client->VoxtaClientCharacterRegisteredEvent.AddDynamic(this, &ThisClass::OnCharacterRegistered);

// Implement handlers
void OnVoxtaStateChanged(VoxtaClientState NewState)
{
    if(NewState == VoxtaClientState::Idle)
    {
        // Connection ready, can start chats
    }
}

void OnCharacterRegistered(const FAiCharData& CharData) 
{
    // Store character info
    FString Name = CharData.GetName();
    FGuid CharId = CharData.GetId();
    // Can use CharId to start chats
}
```

5. Get available characters:
```cpp
// Get array of all available characters
TArray<FAiCharData> Characters = Client->GetAvailableAiCharactersCopy();

// Or find specific character
for(const FAiCharData& Character : Characters)
{
    if(Character.GetName() == "DesiredName")
    {
        // Found character
        StartChatWithCharacter(Character.GetId());
    }
}
```

6. Create chat sessions:
```cpp
// Start chat with optional context
Client->StartChatWithCharacter(CharacterId, "Optional context");

// Listen for chat events 
Client->VoxtaClientChatSessionStartedEvent.AddDynamic(this, &ThisClass::OnChatStarted);
Client->VoxtaClientCharMessageAddedEvent.AddDynamic(this, &ThisClass::OnMessageReceived);

void OnChatStarted(const FChatSession& Session)
{
    // Chat session ready
    // Can now send messages
    Client->SendUserInput("Hello!", true); // true = generate AI reply
}

void OnMessageReceived(const FBaseCharData& Sender, const FChatMessage& Message)
{
    FString Text = Message.GetTextContent();
    // Handle message...
}
```

7. Add audio playback for characters:
```cpp
// Add component to actor
UVoxtaAudioPlayback* AudioComp = NewObject<UVoxtaAudioPlayback>(this);
AudioComp->RegisterComponent();

// Initialize for specific character
AudioComp->Initialize(CharacterId);

// Listen for playback events
AudioComp->VoxtaMessageAudioPlaybackFinishedEvent.AddDynamic(this, &ThisClass::OnAudioFinished);
```

## Events

The module provides extensive event systems for state changes:

- VoxtaClientStateChangedEvent - Connection state changes
- VoxtaClientCharacterRegisteredEvent - New character data loaded
- VoxtaClientCharMessageAddedEvent - New chat messages
- VoxtaClientChatSessionStartedEvent - Chat session lifecycle
- VoxtaClientAudioPlaybackRegisteredEvent - Audio handler registration

## Server Requirements

- Requires a running Voxta server
- Compatible with Voxta API version defined in VoxtaVersionData 
- Supports HTTP/WebSocket communication
- Optional services: TextToSpeech, SpeechToText

## Additional Features

- Global audio fallback system
- Character thumbnail caching
- Automatic reconnection handling
- Debug logging utilities
- Audio format conversion

## Dependencies

- UnrealEngine 
  - `Core`
  - `CoreUObject`
  - `Engine`
  - `Json`
  - `HTTP` 
  - `WebSockets`
  - `Voice`
  - `Projects`
  - `ImageWrapper`
  - `Networking`
  - `RenderCore`
- UnrealVoxta
  - `VoxtaAudioUtility`
  - `VoxtaData`
  - `VoxtaUtility_A2F`
- Third-party
  - `OVRLipSync` (optional)

## Licensing

MIT license - copyright (c) 2025 grrimgrriefer & DZnnah. See LICENSE in root for details.
