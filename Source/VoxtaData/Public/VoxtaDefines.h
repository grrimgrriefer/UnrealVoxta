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

#define TARGETED_SERVER_VERSION TEXT("1.0.0-beta.147")
#define TARGETED_API_VERSION TEXT("2025-01")

/** Helper function for SIGNALR_STRINGVIEW macro */
template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, FStringView>>>
FSignalRValue ConvertToSignalRValue(const T& value)
{
	static_assert(std::is_same<std::decay_t<T>, FStringView>::value, "SIGNALR_STRINGVIEW macro can only be used with "
		"FStringView. Please use SIGNALR_STRING for regular string literals");
	return FSignalRValue(FString(value));
}

VOXTADATA_API DECLARE_LOG_CATEGORY_EXTERN(VoxtaLog, Log, All);

static const FString VOXTA_CONTEXT_KEY = FString("UnrealVoxta - SimpleChat");
static const FString EMPTY_STRING = FString(TEXT(""));

DECLARE_DELEGATE_ThreeParams(FDownloadedTextureDelegateNative, bool, const UTexture2DDynamic*, const FIntVector2&);

/** Helper function to convert FGuid value to a string (forced DigitsWithHyphensLower) */
VOXTADATA_API FString GuidToString(const FGuid& input);
