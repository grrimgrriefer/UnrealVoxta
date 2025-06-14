// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

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
 */
UENUM(BlueprintType, Category = "Voxta")
enum class VoxtaServiceType : uint8
{
	TextGen,
	SpeechToText,
	TextToSpeech
};