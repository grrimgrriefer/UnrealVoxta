// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

struct VoxtaServiceData
{
	enum class ServiceType
	{
		TEXT_GEN,
		SPEECH_TO_TEXT,
		TEXT_TO_SPEECH
	};

	ServiceType m_serviceType;
	FString m_serviceName;
	FString m_serviceId;

	explicit VoxtaServiceData(ServiceType type,
			FString name,
			FString id) :
		m_serviceType(type),
		m_serviceName(name),
		m_serviceId(id)
	{
	}
};
