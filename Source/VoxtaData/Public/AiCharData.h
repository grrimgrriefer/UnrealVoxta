// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "CharDataBase.h"
#include "AiCharData.generated.h"

/// <summary>
/// Data struct that contains all the relevant information for an AI character.
/// </summary>
USTRUCT()
struct FAiCharData : public FCharDataBase
{
	GENERATED_BODY()

public:
	FStringView GetCreatorNotes() const { return m_creatorNotes; };
	bool GetIsAllowedExplicitContent() const { return m_allowedExplicitContent; };
	bool GetIsFavorite() const { return m_isFavorite; };

	explicit FAiCharData(FStringView id,
		FStringView name,
		FStringView creatorNotes,
		bool isExplicitContent,
		bool isFavorite) : FCharDataBase(id, name),
		m_creatorNotes(creatorNotes),
		m_allowedExplicitContent(isExplicitContent),
		m_isFavorite(isFavorite)
	{
	}

	explicit FAiCharData() : FCharDataBase() {}

private:
	FString m_creatorNotes = "";
	bool m_allowedExplicitContent = false;
	bool m_isFavorite = false;
};
