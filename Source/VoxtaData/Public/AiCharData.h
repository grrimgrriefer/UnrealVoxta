// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharData.h"
#include "VoxtaDefines.h"
#include "AiCharData.generated.h"

/**
 * FAiCharData
 * Read-only data struct containing all the relevant information for an AI character.
 * Used by the VoxtaClient to represent AI characters in the chat system.
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
	 * @return Immutable reference of the URL that points to where the image for this character can be retrieved from.
	 */
	virtual FStringView GetThumbnailUrl() const override { return m_thumbnailUrl; }

	/**
	 * Create an instance of the datacontainer for a specific AI character.
	 *
	 * @param id The id (guid in string version) that the VoxtaServer has assigned to this character.
	 * @param name The name of this character, as reported by VoxtaServer
	 * @param creatorNotes The custom creator notes for this character, as reported by VoxtaServer
	 * @param isExplicitContent If VoxtaServer has this character marked for explicit content or not.
	 * @param isFavorite If the user has favorited this specific character in Voxta.
	 * @param thumbnailUrl The URL for the character's thumbnail image.
	 * @param packageId The package ID this character belongs to.
	 * @param packageName The name of the package this character belongs to.
	 */
	explicit FAiCharData(const FGuid& id,
		FStringView name,
		FStringView creatorNotes,
		bool isExplicitContent,
		bool isFavorite,
		FStringView thumbnailUrl,
		const FGuid& packageId,
		FStringView packageName) :
		FBaseCharData(id, name),
		m_creatorNotes(creatorNotes),
		m_allowedExplicitContent(isExplicitContent),
		m_isFavorite(isFavorite),
		m_thumbnailUrl(thumbnailUrl),
		m_packageId(packageId),
		m_packageName(packageName)
	{}

	/** Default constructor. */
	FAiCharData() = default;

	virtual ~FAiCharData() override = default;

	/**
	 * Get whether this character allows explicit content.
	 * @return True if explicit content is allowed for this character.
	 */
	bool GetAllowedExplicitContent() const { return m_allowedExplicitContent; }

	/**
	 * Get whether this character is marked as a favorite by the user.
	 * @return True if this character is favorited.
	 */
	bool GetIsFavorite() const { return m_isFavorite; }

	/**
	 * Get the creator's notes about this character.
	 * @return The creator notes string.
	 */
	FStringView GetCreatorNotes() const { return m_creatorNotes; }

	/**
	 * Get the ID of the package this character belongs to.
	 * @return The package GUID.
	 */
	const FGuid& GetPackageId() const { return m_packageId; }

	/**
	 * Get the name of the package this character belongs to.
	 * @return The package name.
	 */
	FStringView GetPackageName() const { return m_packageName; }
#pragma endregion

#pragma region data
private:
	/** Creator notes for this character. */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Creator notes"))
	FString m_creatorNotes = EMPTY_STRING;

	/** Whether explicit content is allowed for this character. */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Allowed explicit content"))
	bool m_allowedExplicitContent = false;

	/** Whether this character is favorited by the user. */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Is Favorited"))
	bool m_isFavorite = false;

	/** URL to the character's thumbnail image. */
	FString m_thumbnailUrl = EMPTY_STRING;
	/** Package ID this character belongs to. */
	FGuid m_packageId = FGuid();
	/** Name of the package this character belongs to. */
	FString m_packageName = EMPTY_STRING;
#pragma endregion
};
