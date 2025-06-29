// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaServiceType.h"
#include "VoxtaServiceEntryData.generated.h"

/**
 * FVoxtaServiceEntryData
 * Data struct representing a specific serviceEntry provided within the VoxtaServiceGroup. (i.e. Kokoro, Coqui, F5, etc)
 * Contains type, name, and unique identifier for the serviceEntry.
 * 
 */
USTRUCT(BlueprintType, Category = "Voxta")
struct VOXTADATA_API FVoxtaServiceEntryData
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Construct a new VoxtaServiceEntryData.
	 *
	 * @param type The type of the serviceEntry.
	 * @param name The name of the serviceEntry.
	 * @param id The unique identifier for the serviceEntry.
	 */
	explicit FVoxtaServiceEntryData(VoxtaServiceType type,
			FStringView name,
			FGuid id) :
		m_serviceType(type),
		m_serviceName(name),
		m_serviceId(id)
	{}

	/** Default constructor. */
	FVoxtaServiceEntryData() = default;
#pragma endregion

#pragma region data
private:
	/** Whether or not this serviceEntry is enabled. */
	bool m_isEnabled = false;

	/** The type of the serviceEntry (e.g., TextGen, TTS, STT). */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Service type"))
	VoxtaServiceType m_serviceType = VoxtaServiceType::Unknown;

	/** The name of the serviceEntry. */
	UPROPERTY(BlueprintReadOnly, Category = "Voxta", meta = (AllowPrivateAccess = "true", DisplayName = "Service name"))
	FString m_serviceName;

	/** The unique identifier for the serviceEntry. */
	FGuid m_serviceId;
#pragma endregion
};
