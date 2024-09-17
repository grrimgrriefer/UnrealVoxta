// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharData.h"
#include "UserCharData.generated.h"

/**
 * FUserCharData
 * Read-only data struct containing all the relevant information for the character representing the User (player).
 *
 * Note: The User currently has no custom data, just the basic baseCharData data. This might change later on, idk yet.
 */
USTRUCT(BlueprintType, Category = "Voxta")
struct FUserCharData : public FBaseCharData
{
	GENERATED_BODY()

#pragma region public API
public:
	explicit FUserCharData(FStringView id, FStringView name) :
		FBaseCharData(id, name)
	{
	}

	explicit FUserCharData() : FBaseCharData() {}
#pragma endregion
};
