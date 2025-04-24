// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaHelperFunctionLibrary.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Logging/StructuredLog.h"

bool UVoxtaHelperFunctionLibrary::IsIpv4Valid(const FString& address)
{
	const FString trimmedAddress = address.TrimStartAndEnd();
	
	if (trimmedAddress.IsEmpty())
	{
		return false;
	}
	else if (trimmedAddress.ToLower() == LOCALHOST.ToLower())
	{
		return true;
	}
	
	FIPv4Address parsed;
	if (!FIPv4Address::Parse(trimmedAddress, parsed))
	{
		return false;
	}
	return true;
}

FString UVoxtaHelperFunctionLibrary::FloatToStringPrecision(float input, int digitsAfterPoint)
{
	if (FMath::IsNaN(input))
	{
		return TEXT("NaN");
	}
	else if (!FMath::IsFinite(input))
	{
		return input > 0 ? TEXT("Infinity") : TEXT("-Infinity");
	}

	if (digitsAfterPoint < 0)
	{
		UE_LOGFMT(VoxtaLog, Error, "FloatToStringPrecision: Cannot have negative amount of digits (requested: {0} with {1} digits)", input, digitsAfterPoint);
		digitsAfterPoint = 0;
	}

	if (digitsAfterPoint == 0)
	{
		return FString::FromInt(FMath::RoundToInt(input));
	}

	const int64 factor = FMath::RoundToInt64(FMath::Pow(10.f, digitsAfterPoint));
	const int64 rounded = FMath::RoundToInt64(input * factor);

	const int64 wholePart = rounded / factor;
	int64 fractionalPart = rounded > 0 ? rounded % factor : (-1) * (rounded % factor);
	
	return FString::Format(TEXT("{0}.{1}"),
	{
		FString::FromInt(wholePart),
		FString::Printf(TEXT("%0*lld"), digitsAfterPoint, fractionalPart)
	});
}