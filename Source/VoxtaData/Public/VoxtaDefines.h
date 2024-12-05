// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "Logging/LogMacros.h"
#include "type_traits"

class FSignalRValue;

/** Macro to make a FName from a literal without the extra overhead. */
#define EASY_NAME(x) FName(TEXT(x))
/** Macro to make a FString from a literal without the extra overhead. */
#define EASY_STRING(x) FString(TEXT(x))
/** Macro to make a FSignalRValue from a literal without the extra overhead. */
#define SIGNALR_STRING(x) FSignalRValue(FString(TEXT(x)))
/** Macro to make a FSignalRValue from a FStringView the extra overhead. */
#define SIGNALR_STRINGVIEW(x) ConvertToSignalRValue(x)

/** Helper function for SIGNALR_STRINGVIEW macro */
template <typename T>
FSignalRValue ConvertToSignalRValue(const T& value);

DECLARE_LOG_CATEGORY_EXTERN(VoxtaLog, Log, All);

const FString VOXTA_CONTEXT_KEY = FString("UnrealVoxta - SimpleChat");
const FString EMPTY_STRING = FString(TEXT(""));

VOXTADATA_API FString GuidToString(const FGuid& input);