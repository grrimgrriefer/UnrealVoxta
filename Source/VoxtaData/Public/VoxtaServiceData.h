// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

struct VoxtaServiceData
{
public:
	enum class ServiceType
	{
		TextGen,
		SpeechToText,
		TextToSpeech
	};

	const ServiceType m_serviceType;
	const FString m_serviceName;
	const FString m_serviceId;

	explicit VoxtaServiceData(ServiceType type,
			FString name,
			FString id) :
		m_serviceType(type),
		m_serviceName(name),
		m_serviceId(id)
	{
	}
};
