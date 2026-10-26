// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaRuntimeData.generated.h"

USTRUCT(BlueprintType)
struct UNREALVOXTA_API FVoxtaUserInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString m_Id;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString m_Name;
};

USTRUCT(BlueprintType)
struct UNREALVOXTA_API FVoxtaCharacterData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString m_Id;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString m_Name;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString m_CreatorNotes;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool m_IsAllowExplicitContent = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool m_IsFavorite = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString m_ThumbnailUrl;
};

USTRUCT(BlueprintType)
struct UNREALVOXTA_API FVoxtaRuntimeData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVoxtaUserInfo m_User;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FVoxtaCharacterData> m_Characters;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString m_ServerVersion;

	const FVoxtaCharacterData* FindCharacterByName(const FString& characterName) const
	{
		return m_Characters.FindByPredicate([&characterName](const FVoxtaCharacterData& data)
		{
			return data.m_Name.Equals(characterName, ESearchCase::IgnoreCase);
		});
	}

	void Reset()
	{
		m_User = FVoxtaUserInfo();
		m_ServerVersion.Empty();
	}
};
