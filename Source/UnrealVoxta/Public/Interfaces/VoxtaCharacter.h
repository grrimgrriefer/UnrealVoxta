// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaRuntimeData.h"
#include "UObject/Interface.h"
#include "VoxtaCharacter.generated.h"

UINTERFACE()
class UVoxtaCharacter : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface used by plugin code internally to interact with characters.
 *
 * Can be implemented on any class you want to use for characters.
 */
class UNREALVOXTA_API IVoxtaCharacter
{
	GENERATED_BODY()

public:
	virtual void OnRegistered(const FVoxtaCharacterData& VoxtaCharData) {};
	virtual void OnUnregistered() {};
};
