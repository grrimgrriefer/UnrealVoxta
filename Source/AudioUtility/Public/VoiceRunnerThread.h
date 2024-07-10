#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "AudioCaptureHandler.h"

#define VOICE_COMPONENT_NULL 255
#define THREAD_RETURN_DEFAULT_VALUE 254
#define RECEIVE_VOICE_DATA_SUCCESS 0
#define SEND_VOICE_DATA_SUCCESS 1

class FVoiceRunnerThread : public FRunnable
{
public:

	FVoiceRunnerThread(AudioCaptureHandler* InVoiceComponent, float InSleepTime)
		: VoiceComponent(InVoiceComponent), SleepTime(InSleepTime), Thread(nullptr), bStopped(false)
	{
	}

	virtual ~FVoiceRunnerThread() override;

	virtual void Start();
	virtual void Stop() override;
	uint32 Run() override;

protected:

	UPROPERTY()
	AudioCaptureHandler* VoiceComponent;

	float SleepTime;

	FRunnableThread* Thread;

	bool bStopped;
};
