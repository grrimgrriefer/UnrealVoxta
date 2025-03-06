// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaVersionData.generated.h"

USTRUCT(BlueprintType)
struct VOXTADATA_API FVoxtaVersionData
{
	GENERATED_BODY()

#pragma region public API
public:
	FStringView GetCompatibleServerVersion() const { return m_compatibleServerVersion; };

	FStringView GetCompatibleAPIVersion() const { return m_compatibleAPIVersion; };

	FStringView GetServerVersion() const { return m_serverVersion; };

	FStringView GetApiVersion() const { return m_apiVersion; };
	
	bool IsMatchingServerVersion() const { return m_serverVersion == m_compatibleServerVersion; }

	bool IsMatchingAPIVersion() const { return m_apiVersion == m_compatibleAPIVersion; }

	
	explicit FVoxtaVersionData(FString voxtaServerVersion, FString voxtaAPIVersion) :
		m_serverVersion(voxtaServerVersion),
		m_apiVersion(voxtaAPIVersion)
	{}

	explicit FVoxtaVersionData() {};
#pragma endregion

private:
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Supported VoxtaServer version"))
	FString m_compatibleServerVersion = TEXT("1.0.0-beta.132");

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Supported VoxtaServer API version"))
	FString m_compatibleAPIVersion = TEXT("2024-11");
	
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Current VoxtaServer version"))
	FString m_serverVersion = TEXT("not connected");
	
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Current VoxtaServer API version"))
	FString m_apiVersion = TEXT("not connected");
};