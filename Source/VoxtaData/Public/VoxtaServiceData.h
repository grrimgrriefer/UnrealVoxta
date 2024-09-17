// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * VoxtaServiceData
 * Data class containing the relevant information for VoxtaServer services.
 * Currently used as part of the FChatSession data container.
 *
 * Note: There is currently no support for disabling / enabling VoxtaServer services while the conversation is on-going.
 */
struct VoxtaServiceData
{
#pragma region public helper classes
public:
	/**
	 * ServiceType
	 * All the possible VoxtaServer Services that the UnrealVoxta client currently supports.
	 * Used to fetch the correct version of the service from the FChatSession data container.
	 */
	enum class ServiceType : uint8
	{
		TextGen			UMETA(DisplayName = "TextGen"),
		SpeechToText	UMETA(DisplayName = "SpeechToText"),
		TextToSpeech	UMETA(DisplayName = "TextToSpeech")
	};
#pragma endregion

#pragma region public API
	explicit VoxtaServiceData(ServiceType type,
			FStringView name,
			FStringView id) :
		SERVICE_TYPE(type),
		SERVICE_NAME(name),
		SERVICE_ID(id)
	{
	}
#pragma endregion

#pragma region data
	const ServiceType SERVICE_TYPE;
	const FString SERVICE_NAME;
	const FString SERVICE_ID;
#pragma endregion
};
