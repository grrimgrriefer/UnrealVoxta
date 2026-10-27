// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VoxtaClient.generated.h"

struct FVoxtaCharacterData;
struct FVoxtaRuntimeData;
struct FGameplayTag;
struct FConstStructView;
struct FVoxtaUserConfiguration;
enum class VoxtaClientState : uint8;

UINTERFACE()
class UVoxtaClient : public UInterface
{
	GENERATED_BODY()
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnVoxtaCharactersLoaded, TConstArrayView<FVoxtaCharacterData>);

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

	virtual TMulticastDelegate<void(const TConstArrayView<FVoxtaCharacterData>&)>& OnCharactersLoaded() = 0;

	virtual void EnsureConnectionWithServer() = 0;
	virtual bool TrySendFlowEvent(FGameplayTag tag, bool hasPayload, const FConstStructView& payload) = 0;

	virtual const TSet<VoxtaClientState>& GetStates() const = 0;
	virtual const FVoxtaUserConfiguration& GetUserConfiguration() const = 0;
	virtual TConstArrayView<FVoxtaCharacterData> GetAvailableCharacters() const = 0;
};
