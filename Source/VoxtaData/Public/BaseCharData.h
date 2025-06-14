// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaDefines.h"
#include "BaseCharData.generated.h"

/**
 * FBaseCharData
 * Read-only data struct containing all the universal information fields for a character.
 * Used as a base for both AI and user characters.
 * Not directly exposed via public API, but referenced in events and chat messages.
 */
USTRUCT(BlueprintType, Category = "Voxta")
struct VOXTADATA_API FBaseCharData
{
	GENERATED_BODY()

#pragma region public API
public:
	/** @return Immutable reference to the VoxtaServer assigned id of this character. */
	const FGuid& GetId() const { return m_id; }

	/** @return Immutable reference to the name of this character. */
	FStringView GetName() const { return m_name; }

	/**
	 * Derived classes should override this to provide character-specific thumbnails.
	 *
	 * @return URL to the thumbnail image for this character. Empty by default.
	 */
	virtual FStringView GetThumbnailUrl() const PURE_VIRTUAL(FBaseCharData::GetThumbnailUrl, return FString();)

	/**
	 * Create an instance of the data container for the CharData.
	 *
	 * @param id The GUID that the VoxtaServer has assigned to this character.
	 * @param name The name of this character, as reported by VoxtaServer
	 */
	explicit FBaseCharData(FGuid id, FStringView name) :
	m_id(id),
	m_name(name),
	m_idAsString(GuidToString(id))
	{}

	/** Default constructor. */
	FBaseCharData() = default;

	virtual ~FBaseCharData() = default;
#pragma endregion

#pragma region data
protected:
	/** Character ID assigned by VoxtaServer. */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Character ID"))
	FGuid m_id = FGuid();

	/** Name of the character. */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Name"))
	FString m_name = EMPTY_STRING;

	/** Character ID as formatted string. */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Character ID as formatted string"))
	FString m_idAsString = EMPTY_STRING;
#pragma endregion
};
