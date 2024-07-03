// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

class VoxtaAudioInput
{
public:
	explicit VoxtaAudioInput(FStringView serverIP, int serverPort);

	void StartStreaming();
	void StopStreaming();

private:
	AudioCaptureDevice m_audioCaptureDevice;
	TSharedPtr<AudioWebSocket> m_audioWebSocket;

	//std::jthread m_startupThread;
	bool m_isStreaming = false;
	bool m_isStartingUp = false;
};
