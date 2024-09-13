// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * VoxtaLogger
 * Internal class that enables the automatic printing of warnings and errors in the VoxtaLog category, on the
 * top left corner of the screen while inside of the editor.
 */
class VoxtaLogger : public FOutputDevice
{
#pragma region public API
public:
	/** Unregister VoxtaLogger from GLog. I.e. cleanup */
	~VoxtaLogger();

	/** Register VoxtaLogger with GLog, allowing the override to run on any logs printed to the console. */
	void RegisterVoxtaLogger();
#pragma endregion

#pragma region FOutputDevice overrides
protected:
	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const class FName& Category) override;
#pragma endregion

#pragma region data
private:
	const FName m_voxtaLogCategory = TEXT("VoxtaLog");
	const FName m_signalRLogCategory = TEXT("LogSignalR");
	const FName m_httpLogCategory = TEXT("LogHttp");
#pragma endregion
};