# VoxtaData Module

## Overview

The VoxtaData module provides the foundational data structures and type definitions for the UnrealVoxta plugin. It contains read-only & mutable data containers, server response types, state enums, and service definitions that form the core data model for Voxta's chat system.

## Module Structure

### Character Data Models

- `BaseCharData` - Base class for character data structures
- `UserCharData` - User/player character representation
- `AiCharData` - AI character data with additional metadata

### Chat System

- `ChatSession` - Container for active chat session state
- `ChatMessage` - Individual message data structure
- `MessageChunkState` - Message chunk processing states

### Server Response System

- `ServerResponseBase` - Base class for all server responses
- `ServerResponseType` - Supported response types enum
- Specialized response classes for different server events:
  - Welcome/Authentication
  - Chat operations (messages, started, update, etc.)
  - Speech transcription
  - etc.

### Service Management

- `VoxtaServiceType` - Available service types (TextGen, STT, TTS)
- `VoxtaServiceData` - Service configuration container
- `VoxtaServiceStatusType` - Service state tracking

## Thread Safety

The module implements thread-safe data structures where needed:
- `FChatSession` is thread-safe for concurrent read/write access
- `FChatMessage` supports incremental updates from different threads
- Server response objects are always immutable

## Features

### Chat Session Management

```cpp
// Create a new chat session
FChatSession chatSession(
    characterArray,      // TArray<const FAiCharData*>
    chatId,              // FGuid
    sessionId,           // FGuid
    enabledServices,     // TMap<VoxtaServiceType, FVoxtaServiceData>
    contextString        // FStringView
);

// Access chat session data
const TArray<FChatMessage>& messages = chatSession.GetChatMessages();
const TArray<FGuid>& participants = chatSession.GetCharacterIds();
FStringView context = chatSession.GetChatContext();

// Modify chat session
chatSession.AddChatMessage(newMessage);
chatSession.UpdateContext(newContextText);
chatSession.RemoveChatMessage(messageId);
```

### Character Data Handling

```cpp
// Create AI character data
FAiCharData aiChar(
    characterId,        // FGuid
    characterName,      // FStringView
    creatorNotes,       // FStringView 
    isExplicit,         // bool
    isFavorited,        // bool
    thumbnailUrl,       // FStringView
    packageId,          // FGuid
    packageName         // FStringView
);

// Access character data
FGuid id = aiChar.GetId();
FStringView name = aiChar.GetName();
FString thumbnail = aiChar.GetThumbnailUrl();
bool isExplicit = aiChar.GetAllowedExplicitContent();
```

### Message Management

```cpp
// Create a new chat message
FChatMessage message(messageId, senderId);

// Add content incrementally
message.TryAppendMoreContent(textChunk, audioUrlPath);

// Finalize message
message.MarkComplete();

// Check message state
bool isComplete = message.GetIsComplete();
FStringView text = message.GetTextContent();
const TArray<FString>& audioUrls = message.GetAudioUrls();
```

### Version Management

```cpp
// Create version data container
FVoxtaVersionData versionData(serverVersion, apiVersion);

// Check version compatibility
bool isApiCompatible = versionData.IsMatchingAPIVersion();
bool isServerCompatible = versionData.IsMatchingServerVersion();

// Get version information
FStringView targetApi = versionData.GetCompatibleAPIVersion();
FStringView currentApi = versionData.GetApiVersion();
```

## Dependencies

- UnrealEngine
  - `Core`
  - `CoreUObject`

## Licensing

MIT license - copyright (c) 2024 grrimgrriefer & DZnnah. See LICENSE in root for details.
