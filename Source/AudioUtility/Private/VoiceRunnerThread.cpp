// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoiceRunnerThread.h"
#include "HAL/RunnableThread.h"
#include "AudioCaptureHandler.h"

FVoiceRunnerThread::~FVoiceRunnerThread()
{
	if (Thread != nullptr)
	{
		Thread->Kill();
		delete Thread;
	}
}

uint32 FVoiceRunnerThread::Run()
{
	uint32 Result = THREAD_RETURN_DEFAULT_VALUE;
	while (!bStopped)
	{
		FPlatformProcess::Sleep(SleepTime);
		if (VoiceComponent)
		{
			VoiceComponent->CaptureAndSendVoiceData_Implementation();
			Result = SEND_VOICE_DATA_SUCCESS;
		}
		else
		{
			Stop();
			return VOICE_COMPONENT_NULL;
		}
	}
	return Result;
}

void FVoiceRunnerThread::Start()
{
	Thread = FRunnableThread::Create(this, TEXT("RemoteVoiceHandlerThread"));
}

void FVoiceRunnerThread::Stop()
{
	bStopped = true;
}