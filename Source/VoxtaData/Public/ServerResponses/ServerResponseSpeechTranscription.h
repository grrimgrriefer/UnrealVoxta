// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"

/**
 * ServerResponseSpeechTranscription
 * Read-only data struct containing the relevant data of the 'SpeechTranscription' response from the VoxtaServer.
 * Contains the transcribed speech and its state (partial, end, cancelled).
 */
struct ServerResponseSpeechTranscription : public ServerResponseBase
{
#pragma region helper classes
public:
	/** Contains the possible states of the transcribed speech, as reported by the VoxtaClient. */
	enum class TranscriptionState
	{
		Partial,
		End,
		Cancelled
	};
#pragma endregion

#pragma region public API
public:
	/**
	 * Construct a speech transcription response.
	 *
	 * @param transcribedSpeech The transcribed speech text.
	 * @param transcriptionState The state of the transcription.
	 */
	explicit ServerResponseSpeechTranscription(FStringView transcribedSpeech,
			TranscriptionState transcriptionState) : ServerResponseBase(ServerResponseType::SpeechTranscription),
		TRANSCRIPTION_STATE(transcriptionState),
		TRANSCRIBED_SPEECH(transcribedSpeech)
	{}
#pragma endregion

#pragma region data
public:
	/** The state of the transcription (partial, end, cancelled). */
	const TranscriptionState TRANSCRIPTION_STATE;
	/** The transcribed speech text. */
	const FString TRANSCRIBED_SPEECH;
#pragma endregion
};