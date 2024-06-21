// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "CharDataBase.h"
#include "UserCharData.generated.h"

/// <summary>
/// Read-only data struct containing all the relevant information for the
/// character representing the User (player).
/// </summary>
USTRUCT()
struct FUserCharData : public FCharDataBase
{
	GENERATED_BODY()

public:
	explicit FUserCharData(FStringView id, FStringView name) :
		FCharDataBase(id, name)
	{
	}

	explicit FUserCharData() : FCharDataBase() {}
};
