// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * VoxtaServiceData
 * Data class containing the relevant information for VoxtaServer services.
 *
 * Resides in the FChatSession data container.
 * Cannot be fetched by anything yet, as it serves no use at the moment. (TODO: support for runtime enabling/disabling
 * of VoxtaServer Services)
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
		TextGen,
		SpeechToText,
		TextToSpeech
	};
#pragma endregion

#pragma region public API
public:
	/**
	 * Create an instance, containing the data for one specific VoxtaServer service.
	 *
	 * @param type The kind of service is this data tied to.
	 * @param name The name of this service. (hard-coded, NOT from VoxtaServer)
	 * @param id The VoxtaServer assigned id (guid in string format) tied to this service.
	 */
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
public:
	const ServiceType SERVICE_TYPE;
	const FString SERVICE_NAME;
	const FString SERVICE_ID;
#pragma endregion
};
