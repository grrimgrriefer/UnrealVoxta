// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaDefines.h"

FString GuidToString(const FGuid& input)
{
	return input.ToString(EGuidFormats::DigitsWithHyphensLower);
}

DEFINE_LOG_CATEGORY(VoxtaLog);
