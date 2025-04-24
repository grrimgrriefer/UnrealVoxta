// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "type_traits"

static bool isSensitiveLogsCensored = true;
const FString censoredSensitiveLogText = TEXT("****censored****");

/**
 * Logs a message with sensitive information that will be censored if censoring is enabled.
 * @param LogCategory - The log category to use
* @param Verbosity - The verbosity level (e.g., Warning, Error)
 * @param Format - The format string
 * @param Param - The sensitive parameter to log or censor
 */
#define SENSITIVE_LOG_BASIC(LogCategory, Verbosity, Format, Param) \
{ \
    if (!isSensitiveLogsCensored) \
    { \
        UE_LOG(LogCategory, Verbosity, Format, Param); \
    } \
    else \
    { \
        UE_LOG(LogCategory, Verbosity, Format, *censoredSensitiveLogText); \
    } \
}

/**
 * Logs a message with one sensitive parameter that will be censored if censoring is enabled.
 * @param LogCategory - The log category to use
 * @param Verbosity - The verbosity level (e.g., Warning, Error)
* @param Format - The format string
 * @param Param1 - The sensitive parameter to log or censor
 */
#define SENSITIVE_LOG1(LogCategory, Verbosity, Format, Param1) \
{ \
    if (!isSensitiveLogsCensored) \
    { \
        UE_LOGFMT(LogCategory, Verbosity, Format, Param1); \
    } \
    else \
    { \
        UE_LOGFMT(LogCategory, Verbosity, Format, censoredSensitiveLogText); \
    } \
}

/**
 * Logs a message with two sensitive parameters that will be censored if censoring is enabled.
 * @param LogCategory - The log category to use
 * @param Verbosity - The verbosity level (e.g., Warning, Error)
 * @param Format - The format string
 * @param Param1 - First sensitive parameter to log or censor
 * @param Param2 - Second sensitive parameter to log or censor
 */
#define SENSITIVE_LOG2(LogCategory, Verbosity, Format, Param1, Param2) \
{ \
    if (!isSensitiveLogsCensored) \
    { \
        UE_LOGFMT(LogCategory, Verbosity, Format, Param1, Param2); \
    } \
    else \
    { \
        UE_LOGFMT(LogCategory, Verbosity, Format, censoredSensitiveLogText, censoredSensitiveLogText); \
    } \
}

/**
 * Logs a message with three sensitive parameters that will be censored if censoring is enabled.
 * @param LogCategory - The log category to use
 * @param Verbosity - The verbosity level (e.g., Warning, Error)
 * @param Format - The format string
 * @param Param1 - First sensitive parameter to log or censor
 * @param Param2 - Second sensitive parameter to log or censor
 * @param Param3 - Third sensitive parameter to log or censor
 */
#define SENSITIVE_LOG3(LogCategory, Verbosity, Format, Param1, Param2, Param3) \
{ \
    if (!isSensitiveLogsCensored) \
    { \
        UE_LOGFMT(LogCategory, Verbosity, Format, Param1, Param2, Param3); \
    } \
    else \
    { \
        UE_LOGFMT(LogCategory, Verbosity, Format, censoredSensitiveLogText, censoredSensitiveLogText, censoredSensitiveLogText); \
    } \
}