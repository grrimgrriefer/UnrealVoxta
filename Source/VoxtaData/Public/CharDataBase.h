// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "CharDataBase.generated.h"

/// <summary>
/// Base struct that contains all the relevant information for an AI character.
///
/// NOTE: This is not abstract due to USTRUCT, but should be considered as such.
/// </summary>
USTRUCT(BlueprintType)
struct VOXTADATA_API FCharDataBase
{
	GENERATED_BODY()

public:
	FStringView GetId() const { return m_id; }
	FStringView GetName() const { return m_name; }

	explicit FCharDataBase(FStringView id, FStringView name) :
		m_id(id),
		m_name(name)
	{
	}

	explicit FCharDataBase() {};

protected:
	UPROPERTY(BlueprintReadOnly)
	FString m_id;

	UPROPERTY(BlueprintReadOnly)
	FString m_name;
};
