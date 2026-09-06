// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "SubSystems/VoxtaClient.h"
#include "VoxtaStateTreeSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

IVoxtaClient* IVoxtaClient::Get(const UObject* worldContextObject)
{
	if (!worldContextObject)
	{
		return nullptr;
	}
	if (const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(worldContextObject))
	{
		return gameInstance->GetSubsystem<UVoxtaStateTreeSubsystem>();
	}
	return nullptr;
}
