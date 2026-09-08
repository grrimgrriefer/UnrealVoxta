// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPath.h"
#include "VoxtaPluginSettings.generated.h"

/**
 * Global settings for the UnrealVoxta plugin.
 */
UCLASS(config = UnrealVoxta, defaultconfig, meta = (DisplayName = "Unreal Voxta"))
class UNREALVOXTA_API UVoxtaPluginSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	UPROPERTY(EditAnywhere, config, Category = "StateTree", meta = (AllowedClasses = "/Script/StateTreeModule.StateTree"))
	FSoftObjectPath m_stateTreeAsset;
};
