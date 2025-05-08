# VoxtaUtility_A2F Module

## Overview

The VoxtaUtility_A2F module provides integration with NVIDIA's Audio2Face technology for high-quality facial animation from audio. It includes functionality for real-time playback of lip-sync data synchronized with audio, as well as REST-based communication with Audio2Face's headless mode for generating blend shape data.

## Module Structure

### Public API

- `Public/AnimNode_ApplyCustomCurves.h` - Animation node to apply predefined curves to ARKit mapping
- `Public/Audio2FacePlaybackHandler.h` - Handler for synchronizing A2F data with audio playback
- `Public/Audio2FaceRESTHandler.h` - Manages HTTP REST API communication with A2F headless mode
- `Public/LipSyncDataA2F.h` - Wrapper for A2F-lipsync specific data

## Features

### Audio2Face Playback

The module provides a playback system that ensures A2F data is played in sync with the audio component:

```cpp
// Initialize the playback handler
UAudio2FacePlaybackHandler* PlaybackHandler = NewObject<UAudio2FacePlaybackHandler>();
PlaybackHandler->Initialize(AudioComponent);

// Play lip-sync data along with audio
PlaybackHandler->Play(LipSyncData);

// Stop playback when needed
PlaybackHandler->Stop();
```

### Custom Animation Node

The module includes a custom animation node for applying predefined curves to ARKit mapping:

- The node automatically finds the Audio2Face provider on the owning actor.
- Updates happen in PreUpdate() and Evaluate_AnyThread().
- The node applies A2F curve weights to the facial animation rig using ARKIT curve names.


### Audio2Face REST Communication

For generating lip-sync data using Audio2Face's headless mode:

```cpp
// Create a REST handler
TSharedPtr<Audio2FaceRESTHandler> RESTHandler = MakeShared<Audio2FaceRESTHandler>();

// Initialize A2F connection (this is asynchronous)
RESTHandler->TryInitialize();

// Check if A2F is ready
if (RESTHandler->IsAvailable())
{
    // Generate blend shapes from audio file
    RESTHandler->GetBlendshapes(
        "VoiceLine.wav",
        "/Output/Path/",
        "LipSyncData.json",
        [](const FString& Path, bool Success) {
            // Handle result
        }
    );
}
```

### LipSync Data Storage

The module provides a specialized data structure for storing and managing A2F lip-sync data:

```cpp
// Create lip-sync data container (automatically added to root)
ULipSyncDataA2F* LipSyncData = NewObject<ULipSyncDataA2F>();

// Set curve weights (typically from generated data)
LipSyncData->SetA2FCurveWeights(SourceCurves, 30); // at 30 FPS

// Get curve data
const TArray<TArray<float>>& CurveWeights = LipSyncData->GetA2FCurveWeights();
int FPS = LipSyncData->GetFramePerSecond();

// Release data when no longer needed (removes from root)
LipSyncData->ReleaseData();
```

## Dependencies

- UnrealEngine
  - `CoreUObject`
  - `Core`
  - `Engine`
  - `HTTP`
  - `Json`
  - `Projects`
- UnrealVoxta
  - `VoxtaData`

## Licensing

MIT license - copyright (c) 2025 grrimgrriefer & DZnnah. See LICENSE in root for details.
