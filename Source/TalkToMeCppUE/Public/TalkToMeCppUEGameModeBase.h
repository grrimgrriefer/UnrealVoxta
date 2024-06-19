// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TalkToMeCppUEGameModeBase.generated.h"

/// <summary>
/// Barebones AGameModeBase implementation.
/// Ensures that the mouse is visible and that ATestGameCharacter and ATalkToMeCppUeHUD are
/// assigned as defaults.
/// </summary>
UCLASS()
class TALKTOMECPPUE_API ATalkToMeCppUEGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	/// <summary>
	/// Assigns ATestGameCharacter and ATalkToMeCppUeHUD as default classes to spawn.
	/// </summary>
	ATalkToMeCppUEGameModeBase();

	///~ Begin AGameModeBase overrides.
public:
	void StartPlay() override;
	///~ End AGameModeBase overrides.
};
