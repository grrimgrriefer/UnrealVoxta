// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaLogger.h"
#include "Misc/CoreMiscDefines.h"
#include "Logging/LogMacros.h"
#include "Async/Async.h"

#ifndef VOXTA_LOG_DEFINED
DEFINE_LOG_CATEGORY(VoxtaLog);
#define VOXTA_LOG_DEFINED
#endif

VoxtaLogger::~VoxtaLogger()
{
#if WITH_EDITOR
	if (GLog != nullptr)
	{
		GLog->RemoveOutputDevice(this);
	}
#endif
}

void VoxtaLogger::RegisterVoxtaLogger()
{
#if WITH_EDITOR
	check(GLog);
	GLog->AddOutputDevice(this);
#endif
}

void VoxtaLogger::Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const class FName& Category)
{
#if WITH_EDITOR
	if (Category == VOXTA_LOG_CATEGORY)
	{
		FColor color;
		switch (Verbosity)
		{
			case ELogVerbosity::Log:
				return;
			case ELogVerbosity::Warning:
				color = FColor::Orange;
				break;
			default:
				color = FColor::Red;
				break;
		}

		auto LogToScreen = [=] ()
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.0f, color, FString::Format(TEXT("[{0}]: {1} -> {2}"), {
						ToString(Verbosity),
						FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")),
						Message }));
				}
			};

		if (IsInGameThread())
		{
			LogToScreen();
		}
		else
		{
			AsyncTask(ENamedThreads::GameThread, MoveTemp(LogToScreen));
		}
	}
#endif
}