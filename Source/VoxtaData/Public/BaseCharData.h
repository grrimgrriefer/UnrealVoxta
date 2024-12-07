// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaDefines.h"
#include "BaseCharData.generated.h"

/**
 * FBaseCharData
 * Read-only data struct containing all the universal information fields. Can be used by systems who don't need to
 * know if this is an AiCharacer or a UserCharacter representing the User (player).
 *
 * Resides in the UVoxtaClient directly.
 * Cannot be retrieved or fetched through any public API, but will be referenced via the
 * VoxtaClientCharMessageAddedEvent when the user has contributed a message to the chat.
 */
USTRUCT(BlueprintType, Category = "Voxta")
struct VOXTADATA_API FBaseCharData
{
	GENERATED_BODY()

#pragma region public API
public:
	/**  @return Immutable reference to the VoxtaServer assigned id of this character. */
	const FGuid& GetId() const { return m_id; }

	/**  @return Immutable reference to the name of the character who said this message. */
	FStringView GetName() const { return m_name; }

	/**
	 * Create an instance of the datacontainer for the CharData.
	 *
	 * @param id The id (guid in string version) that the VoxtaServer has assigned to this character.
	 * @param name The name of this character, as reported by VoxtaServer
	 */
	explicit FBaseCharData(FGuid id, FStringView name) :
		m_id(id),
		m_name(name),
		m_idAsString(GuidToString(id))
	{}

	/** Default constructor, should not be used manually, but is enforced by Unreal */
	explicit FBaseCharData() {};
#pragma endregion

#pragma region data
private:
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Character ID"))
	FGuid m_id;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Name"))
	FString m_name;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Character ID as formatted string"))
	FString m_idAsString;
#pragma endregion
};
