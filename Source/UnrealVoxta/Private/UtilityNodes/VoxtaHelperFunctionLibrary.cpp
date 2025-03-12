// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#include "Interfaces/IPv4/IPv4Address.h"
#include "UtilityNodes/VoxtaHelperFunctionLibrary.h"

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