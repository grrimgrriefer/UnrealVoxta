// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaLogger.h"
#include "Misc/CoreMiscDefines.h"
#include "Logging/LogMacros.h"

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
#if WITH_EDITOR
	if (Category == m_voxtaLogCategory)
	{
		FColor color;
		switch (Verbosity)
		{
			case ELogVerbosity::Warning:
				color = FColor::Orange;
				break;
			case ELogVerbosity::Error:
				color = FColor::Red;
			default:
				return;
		}

		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.0f, color, FString::Format(*FString(TEXT("[{0}]: {1} -> {2}")), {
			ToString(Verbosity),
			FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")),
			Message }));
#endif
	}
}