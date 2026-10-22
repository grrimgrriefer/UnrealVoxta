// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaRuntimeData.generated.h"

USTRUCT(BlueprintType)
struct UNREALVOXTA_API FVoxtaUserInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxta")
	FString m_Id;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxta")
	FString m_Name;
};

USTRUCT(BlueprintType)
struct UNREALVOXTA_API FVoxtaServicesInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxta")
	FString m_DefaultTextGenId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxta")
	FString m_DefaultSpeechToTextId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxta")
	FString m_DefaultTextToSpeechId;
};

USTRUCT(BlueprintType)
struct UNREALVOXTA_API FVoxtaRuntimeData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxta")
	FVoxtaUserInfo m_User;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxta")
	FVoxtaUserInfo m_Assistant;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxta")
	FVoxtaServicesInfo m_Services;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxta")
	FString m_ServerVersion;

	void Reset()
	{
		m_User = FVoxtaUserInfo();
		m_Assistant = FVoxtaUserInfo();
		m_Services = FVoxtaServicesInfo();
		m_ServerVersion.Empty();
	}
};
