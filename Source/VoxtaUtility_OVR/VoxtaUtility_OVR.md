# VoxtaUtility_OVR Module

## Overview

The VoxtaUtility_OVR module provides integration with Oculus OVR lip-sync technology. It is designed to be optional and excluded from builds if the OVR plugin is not installed in the project.

## Module Structure

### Public API

- `LipSyncDataOVR` - Wrapper for OVR-lipsync specific data implementing ILipSyncBaseData interface

## Features

### OVR LipSync Data Management

The module provides a specialized data structure for storing and managing OVR lip-sync data:

```cpp
// Create lip-sync data container (will be automatically added to root)
ULipSyncDataOVR* LipSyncData = NewObject<ULipSyncDataOVR>();

// Set frame sequence from OVR lip-sync generator
LipSyncData->SetFrameSequence(OVRLipSyncFrameSequence);

// Access the data during playback
UOVRLipSyncFrameSequence* Sequence = LipSyncData->GetOvrLipSyncData();

// Release data and remove from root when no longer needed
LipSyncData->ReleaseData();
```

## Dependencies

- UnrealEngine
  - `Core`
  - `CoreUObject`
- UnrealVoxta
  - `VoxtaData`
- Third-party
  - `OVRLipSync`

## Licensing

MIT license - copyright (c) 2025 grrimgrriefer & DZnnah. See LICENSE in root for details.
