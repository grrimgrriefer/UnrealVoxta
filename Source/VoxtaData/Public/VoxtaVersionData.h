// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaDefines.h"
#include "VoxtaVersionData.generated.h"

/**
 * FVoxtaVersionData
 * Data struct for managing version compatibility between the UnrealVoxta client and VoxtaServer.
 * Contains both the targeted (compatible) versions and current server versions.
 * Used during connection to verify API compatibility and track server version.
 */
USTRUCT(BlueprintType)
struct VOXTADATA_API FVoxtaVersionData
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Get the compatible server version that this client expects.
	 * Used for version compatibility checking during connection.
	 *
	 * @return The expected compatible server version string.
	 */
	FStringView GetCompatibleServerVersion() const { return m_compatibleServerVersion; }

	/**
	 * Get the compatible API version that this client expects.
	 * Used for API compatibility checking during connection.
	 *
	 * @return The expected compatible API version string.
	 */
	FStringView GetCompatibleAPIVersion() const { return m_compatibleAPIVersion; }

	/**
	 * Get the actual server version reported by the connected server.
	 * Used to check version compatibility.
	 *
	 * @return The actual server version string.
	 */
	FStringView GetServerVersion() const { return m_serverVersion; }

	/**
	 * Get the actual API version reported by the connected server.
	 * Used to check API compatibility.
	 *
	 * @return The actual API version string.
	 */
	FStringView GetApiVersion() const { return m_apiVersion; }

	/**
	 * Check if the connected server's version matches the expected version.
	 * Used to validate server compatibility.
	 *
	 * @return True if server version matches expected version.
	 */
	bool IsMatchingServerVersion() const { return m_serverVersion == m_compatibleServerVersion; }

	/**
	 * Check if the connected server's API version matches the expected version.
	 * Used to validate API compatibility.
	 *
	 * @return True if API version matches expected version.
	 */
	bool IsMatchingAPIVersion() const { return m_apiVersion == m_compatibleAPIVersion; }

	explicit FVoxtaVersionData(FString voxtaServerVersion, FString voxtaAPIVersion) :
		m_serverVersion(voxtaServerVersion),
		m_apiVersion(voxtaAPIVersion)
	{}

	/** Default constructor. */
	FVoxtaVersionData() = default;
#pragma endregion

private:
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Supported VoxtaServer version"))
	FString m_compatibleServerVersion = TARGETED_SERVER_VERSION;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Supported VoxtaServer API version"))
	FString m_compatibleAPIVersion = TARGETED_API_VERSION;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Current VoxtaServer version"))
	FString m_serverVersion = TEXT("not connected");

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Current VoxtaServer API version"))
	FString m_apiVersion = TEXT("not connected");
};