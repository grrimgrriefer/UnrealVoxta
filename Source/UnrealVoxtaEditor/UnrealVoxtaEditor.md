# UnrealVoxtaEditor Module

## Overview

The UnrealVoxtaEditor module provides editor-specific functionality for the Voxta plugin in Unreal Engine. It primarily focuses on custom animation tooling, specifically the integration of A2F (Audio-to-Face) curves through a specialized animation graph node.

## Module Structure

### Public API

- `AnimGraphNode_ApplyCustomCurves` - Editor UI node for applying A2F curves in animation blueprints
  - Provides visual interface for configuring A2F curve mappings
  - Integrates with Unreal's animation graph system
  - Handles ARKit blendshape mapping customization

## Features

### Custom Animation Graph Node

The module's primary feature is the `UAnimGraphNode_ApplyCustomCurves` node:

- Integrates with Unreal's Animation Blueprint system
- Provides real-time application of A2F curves to facial animations
- Maps ARKit blendshapes through customizable curves
- Supports runtime modification of curve parameters

### Usage

The animation node can be added to any Animation Blueprint:

- Add the "Apply Custom Curves" node in your Animation Blueprint
- Connect your pose inputs and configure the curve mappings
- The node will automatically handle the ARKit blendshape transformations

## Dependencies

- UnrealEngine
  - `AnimGraph`
  - `BlueprintGraph`
  - `CoreUObject`
  - `Core`
  - `Engine`
- Voxta Components
  - `VoxtaData`
  - `VoxtaUtility_A2F`

## Licensing

MIT license - copyright (c) 2024 grrimgrriefer & DZnnah. See LICENSE in root for details.
