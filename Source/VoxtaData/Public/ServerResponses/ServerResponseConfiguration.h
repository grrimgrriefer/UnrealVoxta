// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "VoxtaData/Public/VoxtaServiceGroupData.h"

/**
 * ServerResponseConfiguration
 * Read-only data struct containing the relevant data of the 'configuration' response from the VoxtaServer.
 * Contains information about enabled Voxta services.
 */
struct ServerResponseConfiguration : public ServerResponseBase
{
#pragma region public API
public:
	/**
	 * Construct a configuration response.
	 *
	 * @param services The map of registered Voxta services (both enabled and disabled).
	 */
	explicit ServerResponseConfiguration(const TArray<FVoxtaServiceGroupData>& services) :
		ServerResponseBase(ServerResponseType::Configuration),
		SERVICES(services)
	{}
#pragma endregion

#pragma region data
public:
	/** The map of all registered service-types (both enabled and disabled), each containing every serviceEntry of that type. */
	const TArray<FVoxtaServiceGroupData> SERVICES;
#pragma endregion
};
