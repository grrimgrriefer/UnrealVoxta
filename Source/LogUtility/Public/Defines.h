// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "Logging/LogMacros.h"
#include "type_traits"

static bool isSensitiveLogsCensored = true;
const FString censoredSensitiveLogText = TEXT("****censored****");

#define SENSITIVE_LOG_BASIC(a,b,c,d) { if (!isSensitiveLogsCensored) { UE_LOG(a,b,c,d); } else { UE_LOG(a,b,c, TEXT("****censored****")); } }

#define SENSITIVE_LOG1(a,b,c,d) { if (!isSensitiveLogsCensored) { UE_LOGFMT(a,b,c,d); } else { UE_LOGFMT(a,b,c, censoredSensitiveLogText); } }
#define SENSITIVE_LOG2(a,b,c,d,e) { if (!isSensitiveLogsCensored) { UE_LOGFMT(a,b,c,d,e); } else { UE_LOGFMT(a,b,c, censoredSensitiveLogText, censoredSensitiveLogText); } }
#define SENSITIVE_LOG3(a,b,c,d,e,f) { if (!isSensitiveLogsCensored) { UE_LOGFMT(a,b,c,d,e,f); } else { UE_LOGFMT(a,b,c, censoredSensitiveLogText, censoredSensitiveLogText, censoredSensitiveLogText); } }
