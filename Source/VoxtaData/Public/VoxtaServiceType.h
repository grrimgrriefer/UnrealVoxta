// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * VoxtaServiceType
 * All the possible VoxtaServer Services that the UnrealVoxta client currently supports.
 * Used to identify and track service types in the chat session.
 *
 * Services:
 * - TextGen: Text generation/chat functionality
 * - SpeechToText: Voice input transcription
 * - TextToSpeech: Voice synthesis for AI responses
 * - ActionInference: Make the AI trigger predefined actions based on the state of an ongoing conversation.
 */
UENUM(BlueprintType, Category = "Voxta")
enum class VoxtaServiceType : uint8
{
	Unknown,
	TextGen,
	SpeechToText,
	TextToSpeech,
	ActionInference
};