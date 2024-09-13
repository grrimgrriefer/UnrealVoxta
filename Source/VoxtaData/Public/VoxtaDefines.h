// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "Logging/LogMacros.h"

#define API_NAME(x) FName(TEXT(x))
#define API_STRING(x) FString(TEXT(x))
#define API_SIGNALR_STRING(x) FSignalRValue(FString(TEXT(x)))
#define API_SIGNALR_STRINGVIEW(x) FSignalRValue(FString(x))

DECLARE_LOG_CATEGORY_EXTERN(VoxtaLog, Log, All);