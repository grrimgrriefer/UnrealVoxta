// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaDefines.h"

/** Helper function for SIGNALR_STRINGVIEW macro */
template <typename T>
FSignalRValue ConvertToSignalRValue(T&& value)
{
	static_assert(std::is_same<std::decay_t<T>, FStringView>::value, "SIGNALR_STRINGVIEW macro can only be used with "
		"FStringView. Please use SIGNALR_STRING for regular string literals");
	return FSignalRValue(FString(value));
}

FString GuidToString(const FGuid& input)
{
	return input.ToString(EGuidFormats::DigitsWithHyphensLower);
}