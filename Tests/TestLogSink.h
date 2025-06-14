// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "VoxtaDefines.h"

class TestLogSink : public FOutputDevice
{
public:
	~TestLogSink()
	{
#if WITH_EDITOR
		if (GLog != nullptr)
		{
			GLog->RemoveOutputDevice(this);
		}
#endif
	}

	void RegisterTestLogSink()
	{
#if WITH_EDITOR
		check(GLog);
		GLog->AddOutputDevice(this);
#endif
	}

	virtual void Serialize(const TCHAR* message, ELogVerbosity::Type verbosity, const class FName& category) override
	{
		m_logMessages.Add(FString(message), verbosity);
	}

	bool ContainsLogMessageWithSubstring(const FGuid& message, ELogVerbosity::Type type) const
	{
		return ContainsLogMessageWithSubstring(GuidToString(message), type);
	}

	bool ContainsLogMessageWithSubstring(const FString& message, ELogVerbosity::Type type) const
	{
		for (const TPair<FString, ELogVerbosity::Type>& entry : m_logMessages)
		{
			if (entry.Key.Contains(message) && entry.Value == type)
			{
				return true;
			}
		}
		return false;
	}
private:
	UPROPERTY()
	TMultiMap<FString, ELogVerbosity::Type> m_logMessages;
};