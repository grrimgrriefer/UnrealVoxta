// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "CharDataBase.generated.h"

/// <summary>
/// Data struct that contains all the relevant information for an AI character.
/// </summary>
USTRUCT()
struct FCharDataBase
{
	GENERATED_BODY()

public:
	FStringView GetId() const { return m_id; }
	FStringView GetName() const { return m_name; }

	explicit FCharDataBase(FStringView id,
		FStringView name) :
		m_id(id),
		m_name(name)
	{
	}

	explicit FCharDataBase() {};

private:
	FString m_id;
	FString m_name;
};
