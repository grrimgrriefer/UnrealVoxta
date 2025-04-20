// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaHelperFunctionLibrary.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Logging/StructuredLog.h"

bool UVoxtaHelperFunctionLibrary::IsIpv4Valid(const FString& address)
{
	FIPv4Address parsed;
	if (address.IsEmpty())
	{
		return false;
	}
	else if (address.ToLower() != LOCALHOST && !FIPv4Address::Parse(address, parsed))
	{
		return false;
	}
	return true;
}

FString UVoxtaHelperFunctionLibrary::FloatToStringPrecision(float input, int digitsAfterPoint)
{
	if (digitsAfterPoint < 0)
	{
		UE_LOGFMT(VoxtaLog, Error, "FloatToStringPrecision: Cannot have negative amount of digits (requested: {0} with {1} digits)", input, digitsAfterPoint);
	}
	if (digitsAfterPoint < 0 || digitsAfterPoint == 0)
	{
		return FString::FromInt(FMath::RoundToInt(input));
	}

	int factor = FMath::RoundToInt(FMath::Pow(10.f, digitsAfterPoint));
	const int64 rounded = FMath::RoundToInt64(input * factor);
	return FString::Format(*FString(TEXT("{0}.{1}")),
		{
			FString::FromInt(rounded / factor),
			FString::FromInt(rounded > 0 ? rounded % factor : (-1) * rounded % factor)
		});
}