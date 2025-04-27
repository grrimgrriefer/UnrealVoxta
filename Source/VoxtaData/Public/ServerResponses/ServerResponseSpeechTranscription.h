// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/**
 * Read-only data struct containing the relevant data of the 'SpeechTranscription' response from the VoxtaServer.
 */
struct ServerResponseSpeechTranscription : public ServerResponseBase
{
#pragma region helper classes
public:
	/** Contains the possible states that of the transcribed speech, as reported by the VoxtaClient. */
	enum class TranscriptionState
	{
		Partial,
		End,
		Cancelled
	};
#pragma endregion

#pragma region public API
public:
	/** Create a deserialized version of the VoxtaServer response represents the 'SpeechTranscription' data. */
	explicit ServerResponseSpeechTranscription(FStringView transcribedSpeech,
			TranscriptionState transcriptionState) : ServerResponseBase(ServerResponseType::SpeechTranscription),
		TRANSCRIPTION_STATE(transcriptionState),
		TRANSCRIBED_SPEECH(transcribedSpeech)
	{}
#pragma endregion

#pragma region data
public:
	const TranscriptionState TRANSCRIPTION_STATE;
	const FString TRANSCRIBED_SPEECH;
#pragma endregion
};