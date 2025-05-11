// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaServiceType.h"
#include "VoxtaServiceData.generated.h"

/**
 * FVoxtaServiceData
 * Data struct representing a single Voxta service (e.g., TextGen, TTS, STT).
 * Contains type, name, and unique identifier for the service.
 *
 * Resides in the FChatSession data container.
 * Cannot be fetched by anything yet, as it serves no use at the moment. (TODO: support for runtime enabling/disabling
 * of VoxtaServer Services)
 */
USTRUCT(BlueprintType, Category = "Voxta")
struct VOXTADATA_API FVoxtaServiceData
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Construct a new VoxtaServiceData.
	 *
	 * @param type The type of the service.
	 * @param name The name of the service.
	 * @param id The unique identifier for the service.
	 */
	explicit FVoxtaServiceData(VoxtaServiceType type,
			FStringView name,
			FGuid id) :
		m_serviceType(type),
		m_serviceName(name),
		m_serviceId(id)
	{}

	/** Default constructor. */
	FVoxtaServiceData() = default;
#pragma endregion

#pragma region data
private:
	/** The type of the service (e.g., TextGen, TTS, STT). */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Service type"))
	VoxtaServiceType m_serviceType;

	/** The name of the service. */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Service name"))
	FString m_serviceName;

	/** The unique identifier for the service. */
	FGuid m_serviceId;
#pragma endregion
};
