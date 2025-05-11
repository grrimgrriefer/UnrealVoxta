// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

/**
 * VoxtaLogger
 * Internal class that enables the automatic printing of warnings and errors in the VoxtaLog category, on the
 * top left corner of the screen while inside of the editor.
 *
 * Note: Only works inside the editor, does nothing in packaged builds.
 */
class VoxtaLogger : public FOutputDevice
{
#pragma region public API
public:
	/** Destructor. Unregisters VoxtaLogger from GLog. */
	~VoxtaLogger();

	/** Register VoxtaLogger with GLog, allowing the override to run on any logs printed to the console. */
	void RegisterVoxtaLogger();
#pragma endregion

#pragma region FOutputDevice overrides
protected:
	/** Custom output implementation, triggered for every single message that is logged, while registered. */
	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const class FName& Category) override;
#pragma endregion

#pragma region data
private:
	const FName VOXTA_LOG_CATEGORY = TEXT("VoxtaLog");
	const FName SIGNALR_LOG_CATEGORY = TEXT("LogSignalR");
	const FName HTTP_LOG_CATEGORY = TEXT("LogHttp");
#pragma endregion
};