// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaServiceType.h"
#include "VoxtaServiceGroupData.generated.h"

struct FVoxtaServiceEntryData;

/**
 * FVoxtaServiceGroupData
 * Data struct representing a group of services for one specific service (e.g., TextGen, TTS, STT).
 * Contains global 'enabled' flag, along with the ID of the default service and a wrapper containing all registered services of this type.
 * 
 */
USTRUCT(BlueprintType, Category = "Voxta")
struct VOXTADATA_API FVoxtaServiceGroupData
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Construct a new VoxtaServiceGroupData.
	 *
	 * @param type The type of the serviceGroup.
	 * @param id The identifier of the default serviceEntry to be used for this serviceGroup.
	 * @param serviceEntries The array that contains all entries  for the ServiceType of this group.
	 */
	explicit FVoxtaServiceGroupData(VoxtaServiceType type,
			FGuid defaultServiceEntryid,
			const TArray<FVoxtaServiceEntryData>& serviceEntries) :
		m_serviceGroupType(type),
		m_serviceEntries(serviceEntries),
		m_defaultServiceEntryId(defaultServiceEntryid)
	{}

	/** Default constructor. */
	FVoxtaServiceGroupData() = default;
#pragma endregion

#pragma region data
private:
	/** Whether or not this service is enabled. */
	bool m_isEnabled = false;

	/** The type of the service (e.g., TextGen, TTS, STT). */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Service type"))
	VoxtaServiceType m_serviceGroupType = VoxtaServiceType::Unknown;

	/** The name of the service. */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Service name"))
	TArray<FVoxtaServiceEntryData> m_serviceEntries;

	/** The identifier of the default serviceEntry to be used for this serviceGroup. */
	FGuid m_defaultServiceEntryId;
#pragma endregion
};
