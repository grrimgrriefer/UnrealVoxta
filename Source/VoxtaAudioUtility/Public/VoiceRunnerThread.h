// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "HAL/ThreadSafeBool.h"

#define VOICE_COMPONENT_NULL 255
#define THREAD_RETURN_DEFAULT_VALUE 254
#define RECEIVE_VOICE_DATA_SUCCESS 0
#define SEND_VOICE_DATA_SUCCESS 1

class AudioCaptureHandler;
class FRunnableThread;

/**
 * FVoiceRunnerThread
 * Logic that runs on a background-thread and will trigger the AudioCaptureHandler repeatedly on a fixed timeinterval
 * to ensure it will send whatever was captured in the timestep to be sent to VoxtaServer microphone input socket.
 *
 * Note: keep in mind to double check locks when using/changing anything this touches
 */
class FVoiceRunnerThread : public FRunnable
{
#pragma region public API
public:
	/**
	 * Register values for the runner but does not start it yet.
	 *
	 * @param voiceComponent The component that will receive the 'CaptureAndSendVoiceData' signal.
	 * @param sleepTime The time (in seconds) that should be waited between triggers.
	 */
	FVoiceRunnerThread(AudioCaptureHandler* voiceComponent, float sleepTime);

	/**
	 * Create & start a new background thread, triggering 'CaptureAndSendVoiceData' on the provided
	 * voice component repeatedly, given the provided timestep.
	 */
	void Start();
#pragma endregion

#pragma region FRunnable overrides
public:
	/** Virtual destructor */
	virtual ~FVoiceRunnerThread() override;

	/**
	 * Stops the runnable object.
	 *
	 * This is called if a thread is requested to terminate early.
	 * @see Init, Run, Exit
	 */
	virtual void Stop() override;

	/**
	 * Runs the runnable object.
	 *
	 * This is where all per object thread work is done. This is only called if the initialization was successful.
	 *
	 * @return The exit code of the runnable object
	 * @see Init, Stop, Exit
	 */
	uint32 Run() override;
#pragma endregion

#pragma region data
private:
	UPROPERTY()
	AudioCaptureHandler* m_voiceComponent;
	FRunnableThread* m_thread;

	float m_sleepTime;
	FThreadSafeBool  m_isStopped;
#pragma endregion
};
