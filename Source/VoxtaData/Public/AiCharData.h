// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharData.h"
#include "AiCharData.generated.h"

/**
 * FUserCharData
 * Read-only data struct containing all the relevant information for an AI character.
 *
 * Resides in the UVoxtaClient directly.
 * Cannot be retrieved or fetched through any public API, but will be referenced via the
 * VoxtaClientCharMessageAddedEvent when the user has contributed a message to the chat.
 */
USTRUCT(BlueprintType)
struct VOXTADATA_API FAiCharData : public FBaseCharData
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Create an instance of the datacontainer for a specific AI character.
	 *
	 * @param id The id (guid in string version) that the VoxtaServer has assigned to this character.
	 * @param name The name of this character, as reported by VoxtaServer
	 * @param creatorNotes The custom creator notes for this character, as reported by VoxtaServer
	 * @param isExplicitContent If VoxtaServer has this character marked for explicit content or not.
	 * @param isFavorite If the user has favorited this specific character in Voxta.
	 */
	explicit FAiCharData(FStringView id,
		FStringView name,
		FStringView creatorNotes,
		bool isExplicitContent,
		bool isFavorite) :
		FBaseCharData(id, name),
		m_creatorNotes(creatorNotes),
		m_allowedExplicitContent(isExplicitContent),
		m_isFavorite(isFavorite)
	{
	}

	/** Default constructor, should not be used manually, but is enforced by Unreal */
	explicit FAiCharData() : FBaseCharData() {}
#pragma endregion

#pragma region data
private:
	// TODO: none of these are used atm, consider removal of the fields
	FString m_creatorNotes = "";
	bool m_allowedExplicitContent = false;
	bool m_isFavorite = false;
#pragma endregion
};
