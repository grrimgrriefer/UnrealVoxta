// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "CharData.generated.h"

USTRUCT()
struct FCharData
{
	GENERATED_BODY()

	FString m_id = "";
	FString m_name = "";
	FString m_creatorNotes = "";
	bool m_explicitContent = false;
	bool m_favorite = false;

	FCharData() {}

	explicit FCharData(FStringView id,
		FStringView name) :
		m_id(id),
		m_name(name)
	{
	}
};
