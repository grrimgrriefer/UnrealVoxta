// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

namespace SensitiveLogging
{
    LOGUTILITY_API extern bool isSensitiveLogsCensored;
    const FString CENSORED_TEXT_CONTENT = TEXT("****censored****");
}

/**
 * Logs a message with sensitive information that will be censored if censoring is enabled.
 * 
 * @param LogCategory - The log category to use
 * @param Verbosity - The verbosity level (e.g., Warning, Error)
 * @param Format - The format string
 * @param Param - The sensitive parameter to log or censor
 */
#define SENSITIVE_LOG_BASIC(LogCategory, Verbosity, Format, Param) \
{ \
    if (!SensitiveLogging::isSensitiveLogsCensored) \
    { \
        UE_LOG(LogCategory, Verbosity, Format, Param); \
    } \
    else \
    { \
        UE_LOG(LogCategory, Verbosity, Format, *SensitiveLogging::CENSORED_TEXT_CONTENT); \
    } \
}

/**
 * Logs a message with one sensitive parameter that will be censored if censoring is enabled.
 * 
 * @param LogCategory - The log category to use
 * @param Verbosity - The verbosity level (e.g., Warning, Error)
 * @param Format - The format string
 * @param Param1 - The sensitive parameter to log or censor
 */
#define SENSITIVE_LOG1(LogCategory, Verbosity, Format, Param1) \
{ \
    if (!SensitiveLogging::isSensitiveLogsCensored) \
    { \
        UE_LOGFMT(LogCategory, Verbosity, Format, Param1); \
    } \
    else \
    { \
        UE_LOGFMT(LogCategory, Verbosity, Format, SensitiveLogging::CENSORED_TEXT_CONTENT); \
    } \
}

/**
 * Logs a message with two sensitive parameters that will be censored if censoring is enabled.
 * 
 * @param LogCategory - The log category to use
 * @param Verbosity - The verbosity level (e.g., Warning, Error)
 * @param Format - The format string
 * @param Param1 - First sensitive parameter to log or censor
 * @param Param2 - Second sensitive parameter to log or censor
 */
#define SENSITIVE_LOG2(LogCategory, Verbosity, Format, Param1, Param2) \
{ \
    if (!SensitiveLogging::isSensitiveLogsCensored) \
    { \
        UE_LOGFMT(LogCategory, Verbosity, Format, Param1, Param2); \
    } \
    else \
    { \
        UE_LOGFMT(LogCategory, Verbosity, Format, SensitiveLogging::CENSORED_TEXT_CONTENT, SensitiveLogging::CENSORED_TEXT_CONTENT); \
    } \
}

/**
 * Logs a message with three sensitive parameters that will be censored if censoring is enabled.
 * 
 * @param LogCategory - The log category to use
 * @param Verbosity - The verbosity level (e.g., Warning, Error)
 * @param Format - The format string
 * @param Param1 - First sensitive parameter to log or censor
 * @param Param2 - Second sensitive parameter to log or censor
 * @param Param3 - Third sensitive parameter to log or censor
 */
#define SENSITIVE_LOG3(LogCategory, Verbosity, Format, Param1, Param2, Param3) \
{ \
    if (!SensitiveLogging::isSensitiveLogsCensored) \
    { \
        UE_LOGFMT(LogCategory, Verbosity, Format, Param1, Param2, Param3); \
    } \
    else \
    { \
        UE_LOGFMT(LogCategory, Verbosity, Format, SensitiveLogging::CENSORED_TEXT_CONTENT, SensitiveLogging::CENSORED_TEXT_CONTENT, SensitiveLogging::CENSORED_TEXT_CONTENT); \
    } \
}