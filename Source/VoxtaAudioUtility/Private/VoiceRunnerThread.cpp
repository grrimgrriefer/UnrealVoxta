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
		m_thread->Kill();
		delete m_thread;
	}
}

uint32 FVoiceRunnerThread::Run()
{
	uint32 Result = THREAD_RETURN_DEFAULT_VALUE;
	FPlatformProcess::Sleep(m_sleepTime);

	while (!m_isStopped)
	{
		if (m_voiceComponent)
		{
			m_voiceComponent->CaptureAndSendVoiceData();
			Result = SEND_VOICE_DATA_SUCCESS;
			FPlatformProcess::Sleep(m_sleepTime);
		}
		else
		{
			UE_LOGFMT(VoxtaLog, Warning, "AudioComponent pointer of FVoiceRunnerThread was destroyed? Stopping thread.");
			m_isStopped = true;
			return VOICE_COMPONENT_NULL;
		}
	}
	UE_LOGFMT(VoxtaLog, Log, "Stopped FVoiceRunnerThread.");
	return Result;
}

void FVoiceRunnerThread::Start()
{
	if (m_thread != nullptr)
	{
		UE_LOGFMT(VoxtaLog, Error, "FVoiceRunnerThread cannot be recycled, please destroy the old instance and make"
			"a new one. Aborting start of background thread.");
		return;
	}
	m_thread = FRunnableThread::Create(this, TEXT("VoiceRunnerThread"));
}

void FVoiceRunnerThread::Stop()
{
	UE_LOGFMT(VoxtaLog, Log, "Registered request to stop FVoiceRunnerThread...");
	m_isStopped = true;
}