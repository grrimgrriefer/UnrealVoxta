// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

struct CharData
{
	FString m_id;
	FString m_name;
	FString m_creatorNotes = "";
	bool m_explicitContent = false;
	bool m_favorite = false;

	explicit CharData(FStringView id,
		FStringView name) :
		m_id(id),
		m_name(name)
	{
	}
};

struct CharDataIdComparer
{
	FStringView m_charId;

	explicit CharDataIdComparer(FStringView charId) :
		m_charId(charId)
	{
	}

	bool operator()(const TUniquePtr<const CharData>& element) const
	{
		return element->m_id == m_charId;
	}
};
