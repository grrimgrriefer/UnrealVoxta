// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoiceRunnerThread.h"
#include "HAL/RunnableThread.h"
#include "AudioCaptureHandler.h"

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
	while (!m_isStopped)
	{
		FPlatformProcess::Sleep(m_sleepTime);
		if (m_voiceComponent)
		{
			m_voiceComponent->CaptureAndSendVoiceData();
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
	m_thread = FRunnableThread::Create(this, TEXT("VoiceRunnerThread"));
}

void FVoiceRunnerThread::Stop()
{
	m_isStopped = true;
}