// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include <Logging/StructuredLog.h>
#include "Misc/AssertionMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(VoxtaLog, Log, All);

class VoxtaLogger : public FOutputDevice
{
public:
	~VoxtaLogger();

	/// <summary>
	/// Add the VoxtaLog to the GlobalLogSingleton.
	/// </summary>
	void RegisterVoxtaLogger();

	///~ Begin FOutputDevice overrides.
protected:
	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const class FName& Category) override;
	///~ End FOutputDevice overrides.

private:
	const FName m_voxtaLogCategory = TEXT("VoxtaLog");
	const FName m_signalRLogCategory = TEXT("LogSignalR");
	const FName m_httpLogCategory = TEXT("LogHttp");
};