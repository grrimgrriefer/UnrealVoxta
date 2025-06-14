// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoiceRunnerThread.h"
#include "VoxtaDefines.h"
#include "HAL/RunnableThread.h"
#include "AudioCaptureHandler.h"
#include "Logging/StructuredLog.h"

FVoiceRunnerThread::FVoiceRunnerThread(AudioCaptureHandler* voiceComponent, float sleepTime) :
	m_voiceComponent(voiceComponent),
	m_thread(nullptr),
	m_sleepTime(sleepTime),
	m_isStopped(false)
{
}

FVoiceRunnerThread::~FVoiceRunnerThread()
{
	if (m_thread != nullptr)
	{
		Stop();
		m_thread->WaitForCompletion();
		delete m_thread;
	}
}

uint32 FVoiceRunnerThread::Run()
{
	uint32 Result = VOICE_RUNNER_ERROR_DEFAULT;
	FPlatformProcess::Sleep(m_sleepTime);

	while (!m_isStopped)
	{
		double startTime = FPlatformTime::Seconds();
		if (m_voiceComponent)
		{
			m_voiceComponent->CaptureAndSendVoiceData();
			Result = VOICE_RUNNER_SUCCESS_SEND;
			double elapsedTime = FPlatformTime::Seconds() - startTime;
			double remainingTime = m_sleepTime - elapsedTime;
			if (remainingTime > 0)
			{
				FPlatformProcess::Sleep(remainingTime);
			}
		}
		else
		{
			UE_LOGFMT(VoxtaLog, Warning, "AudioComponent pointer of FVoiceRunnerThread was destroyed? Stopping thread.");
			m_isStopped = true;
			return VOICE_RUNNER_ERROR_COMPONENT_NULL;
		}
	}
	UE_LOGFMT(VoxtaLog, Log, "Stopped FVoiceRunnerThread.");
	return Result;
}

void FVoiceRunnerThread::Start()
{
	if (m_thread != nullptr || m_isStopped)
	{
		UE_LOGFMT(VoxtaLog, Error, "FVoiceRunnerThread cannot be recycled, please destroy the old instance and make "
			"a new one. Aborting start of background thread.");
		return;
	}
	m_thread = FRunnableThread::Create(this, TEXT("VoiceRunnerThread"), 0, EThreadPriority::TPri_AboveNormal);
	if (!m_thread)
	{
		UE_LOGFMT(VoxtaLog, Error, "Failed to launch FVoiceRunnerThread, FRunnableThread::Create returned nullptr");
		return;
	}
}

void FVoiceRunnerThread::Stop()
{
	UE_LOGFMT(VoxtaLog, Log, "Registered request to stop FVoiceRunnerThread...");
	m_isStopped = true;
}