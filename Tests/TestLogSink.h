// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "VoxtaDefines.h"

class TestLogSink : public FOutputDevice
{
public:
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
			if (entry.Key.Contains(message))
			{
				return true;
			}
		}
		return false;
	}
private:
	TMap<FString, ELogVerbosity::Type> m_logMessages;
};