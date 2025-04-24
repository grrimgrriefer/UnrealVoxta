// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "Logging/LogMacros.h"
#include "type_traits"

class FSignalRValue;
class UTexture2DDynamic;

/** Macro to make a FName from a literal without the extra overhead. */
#define EASY_NAME(x) FName(TEXT(x))
/** Macro to make a FString from a literal without the extra overhead. */
#define EASY_STRING(x) FString(TEXT(x))
/** Macro to make a FSignalRValue from a literal without the extra overhead. */
#define SIGNALR_STRING(x) FSignalRValue(FString(TEXT(x)))
/** Macro to make a FSignalRValue from a FStringView the extra overhead. */
#define SIGNALR_STRINGVIEW(x) ConvertToSignalRValue(x)

/** Helper function for SIGNALR_STRINGVIEW macro */
template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, FStringView>>>
FSignalRValue ConvertToSignalRValue(const T& value);

DECLARE_LOG_CATEGORY_EXTERN(VoxtaLog, Log, All);

static const FString VOXTA_CONTEXT_KEY = FString("UnrealVoxta - SimpleChat");
static const FString EMPTY_STRING = FString(TEXT(""));

DECLARE_DELEGATE_ThreeParams(FDownloadedTextureDelegateNative, bool, const UTexture2DDynamic*, const FIntVector2&);

VOXTADATA_API FString GuidToString(const FGuid& input);

#define TARGETED_SERVER_VERSION TEXT("1.0.0-beta.132");
#define TARGETED_API_VERSION TEXT("2024-11");