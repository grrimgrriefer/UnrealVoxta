// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharData.h"
#include "Containers/UnrealString.h"
#include "UserCharData.generated.h"

/**
 * FUserCharData
 * Read-only data struct containing all the relevant information for the character representing the User (player).
 * Contains basic character data like ID and name, plus a special thumbnail URL format for user profiles.
 *
 * Resides in the UVoxtaClient directly and is initialized during the Welcome response.
 * Referenced in VoxtaClientCharMessageAddedEvent when the user contributes a message.
 */
USTRUCT(BlueprintType, Category = "Voxta")
struct FUserCharData : public FBaseCharData
{
	GENERATED_BODY()

#pragma region public API
public:
	virtual FStringView GetThumbnailUrl() const override { return m_userThumbnailUrl; }

	/**
	 * Create an instance of the datacontainer for the User.
	 *
	 * @param id The id that the VoxtaServer has assigned to the User.
	 * @param name The name of the User, as reported by VoxtaServer
	 */
	explicit FUserCharData(FGuid id, FStringView name) :
		FBaseCharData(id, name),
		m_userThumbnailUrl(FString::Format(*FString(TEXT("/api/profile/{0}/thumbnail")), { GuidToString(id) }))
	{}

	/** Default constructor */
	FUserCharData() = default;

	virtual ~FUserCharData() override = default;
#pragma endregion

#pragma region data
	FString m_userThumbnailUrl;
#pragma endregion
};
