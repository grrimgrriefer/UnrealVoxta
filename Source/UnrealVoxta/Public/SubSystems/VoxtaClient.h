// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VoxtaClient.generated.h"

struct FVoxtaUserConfiguration;

UINTERFACE()
class UVoxtaClient : public UInterface
{
	GENERATED_BODY()
};

/**
 * Lower-level subsystem for Voxta integration.
 *
 * Can be used if you don't intend on using APawns to map VoxtaCharacters onto via the UCharacterSubsystem.
 */
class UNREALVOXTA_API IVoxtaClient
{
	GENERATED_BODY()

public:
	static IVoxtaClient* Get(const UObject* worldContextObject);

	virtual const FVoxtaUserConfiguration& GetVoxtaUserConfiguration() const;
	virtual void EnsureConnectionWithServer() const = 0;
};
