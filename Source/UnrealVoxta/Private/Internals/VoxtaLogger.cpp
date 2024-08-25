// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaLogger.h"

DEFINE_LOG_CATEGORY(VoxtaLog);

VoxtaLogger::~VoxtaLogger()
{
	if (GLog != nullptr)
	{
		GLog->RemoveOutputDevice(this);
	}
}

void VoxtaLogger::RegisterVoxtaLogger()
{
	check(GLog);
	GLog->AddOutputDevice(this);
}

void VoxtaLogger::Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const class FName& Category)
{
	if (Category == m_voxtaLogCategory || Category == m_signalRLogCategory || Category == m_httpLogCategory)
	{
		FColor color;
		switch (Verbosity)
		{
			case ELogVerbosity::Warning:
				color = FColor::Orange;
				break;
			case ELogVerbosity::Display:
			case ELogVerbosity::Log:
				color = FColor::Cyan;
				if (Category == m_signalRLogCategory || Category == m_httpLogCategory)
				{
					return;
				}
				break;
			case ELogVerbosity::Error:
			default:
				color = FColor::Red;
				break;
		}

		GEngine->AddOnScreenDebugMessage(-1, 15.0f, color, FString::Format(*FString(TEXT("[{0}]: {1} -> {2}")), {
			ToString(Verbosity),
			FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")),
			Message }));
	}
}