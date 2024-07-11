// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "VoxtaData/Public/VoxtaServiceData.h"

/// <summary>
/// Read-only data struct containing the relevant data of the 'chatStarted' response
/// from the VoxtaServer.
/// </summary>
struct ServerResponseSpeechTranscription : public IServerResponseBase
{
public:
	enum class TranscriptionState
	{
		PARTIAL,
		END,
		CANCELLED
	};

	const TranscriptionState m_transcriptionState;
	const FString m_transcribedSpeech;

	explicit ServerResponseSpeechTranscription(FStringView transcribedSpeech,
			TranscriptionState transcriptionState) :
		m_transcriptionState(transcriptionState),
		m_transcribedSpeech(transcribedSpeech)
	{
	}

	ServerResponseType GetType() const final
	{
		return ServerResponseType::SpeechTranscription;
	}
};