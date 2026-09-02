// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * Contains information that is specific to the User's own setup of
 * their local VoxtaServer.
 */
struct UNREALVOXTA_API VoxtaUserConfiguration
{
public:
	FString m_VoxtaServerIpv4 = TEXT("127.0.0.1");
	uint16 m_VoxtaServerPort = 5384;
};
