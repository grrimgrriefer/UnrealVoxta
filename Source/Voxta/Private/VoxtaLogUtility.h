// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include <Logging/StructuredLog.h>
#include "Misc/AssertionMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(VoxtaLog, Log, All);

class VoxtaLogUtility : public FOutputDevice
{
public:
	void AddDevice()
	{
		check(GLog);
		GLog->AddOutputDevice(this);
	}

	~VoxtaLogUtility()
	{
		if (GLog != nullptr)
		{
			GLog->RemoveOutputDevice(this);
		}
	}

protected:
	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const class FName& Category) override
	{
		if (Category == m_voxtaLogCategory)
		{
			FDateTime CurrentDateTime = FDateTime::Now();
			FString DateString = CurrentDateTime.ToString();

			FString CustomDateString = CurrentDateTime.ToString(TEXT("%Y-%m-%d %H:%M:%S"));

			FString MyStringPrintf = FString(TEXT("[{0}]: {1} -> {2}"));
			FString MyStringFormatted = FString::Format(*MyStringPrintf, { ToString(Verbosity), CustomDateString, Message });

			FColor color = FColor::Cyan;

			switch (Verbosity)
			{
				case ELogVerbosity::Warning:
					color = FColor::Orange;
					break;
				case ELogVerbosity::Display:
				case ELogVerbosity::Log:
					color = FColor::Cyan;
					break;
				case ELogVerbosity::Error:
				default:
					color = FColor::Red;
					break;
			}

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, color, MyStringFormatted);
		}
	}

private:
	FName m_voxtaLogCategory = TEXT("VoxtaLog");
};