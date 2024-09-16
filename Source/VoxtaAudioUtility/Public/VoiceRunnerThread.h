// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

#define VOICE_COMPONENT_NULL 255
#define THREAD_RETURN_DEFAULT_VALUE 254
#define RECEIVE_VOICE_DATA_SUCCESS 0
#define SEND_VOICE_DATA_SUCCESS 1

class AudioCaptureHandler;

/// <summary>
/// FRunnable that will trigger the AudioCaptureHandler to send data on a fixed timeinterval.
/// Note: This will run on its own thread, so any functionality used must be threadsafe.
/// </summary>
class FVoiceRunnerThread : public FRunnable
{
public:
	/// <summary>
	/// Register the pointer to the AudioCaptureHandler and set the sleepTime interval (in milliseconds)
	/// </summary>
	FVoiceRunnerThread(AudioCaptureHandler* voiceComponent, float sleepTime);

	/// <summary>
	/// Start the thread that will have this runnable assigned to it.
	/// </summary>
	void Start();

	///~ Begin FRunnable overrides.
public:
	virtual ~FVoiceRunnerThread() override;
	virtual void Stop() override;
	uint32 Run() override;
	///~ End FRunnable overrides.

private:
	UPROPERTY()
	AudioCaptureHandler* m_voiceComponent;
	FRunnableThread* m_thread;

	float m_sleepTime;
	bool m_isStopped;
};
