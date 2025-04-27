// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaServiceType.h"
#include "VoxtaServiceData.generated.h"

/**
 * VoxtaServiceData
 * Data class containing the relevant information for VoxtaServer services.
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
	 * Create an instance, containing the data for one specific VoxtaServer service.
	 *
	 * @param type The kind of service is this data tied to.
	 * @param name The name of this service. (hard-coded, NOT from VoxtaServer)
	 * @param id The GUID assigned by the VoxtaServer for this service.
	 */
	explicit FVoxtaServiceData(VoxtaServiceType type,
			FStringView name,
			FGuid id) :
		m_serviceType(type),
		m_serviceName(name),
		m_serviceId(id)
	{}

	explicit FVoxtaServiceData() {}
#pragma endregion

#pragma region data
private:
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Service type"))
	VoxtaServiceType m_serviceType;

	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Service name"))
	FString m_serviceName;

	FGuid m_serviceId;
#pragma endregion
};
