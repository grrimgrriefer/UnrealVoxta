// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharData.h"
#include "UserCharData.generated.h"

/**
 * FUserCharData
 * Read-only data struct containing all the relevant information for the character representing the User (player).
 *
 * Resides in the UVoxtaClient directly.
 * Cannot be retrieved or fetched through any public API, but will be referenced via the
 * VoxtaClientCharMessageAddedEvent when the user has contributed a message to the chat.
 */
USTRUCT(BlueprintType, Category = "Voxta")
struct FUserCharData : public FBaseCharData
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Create an instance of the datacontainer for the User.
	 *
	 * @param id The id (guid in string version) that the VoxtaServer has assigned to the User.
	 * @param name The name of the User, as reported by VoxtaServer
	 */
	explicit FUserCharData(FStringView id, FStringView name) :
		FBaseCharData(id, name)
	{
	}

	/** Default constructor, should not be used manually, but is enforced by Unreal */
	explicit FUserCharData() : FBaseCharData() {}
#pragma endregion
};
