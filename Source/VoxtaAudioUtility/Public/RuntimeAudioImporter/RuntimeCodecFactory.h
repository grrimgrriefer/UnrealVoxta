// Georgy Treshchev 2024.

#pragma once

#include "BaseRuntimeCodec.h"

/**
 * A factory for constructing the codecs used for encoding and decoding audio data
 * Codecs are intended to be registered as modular features
 * 
 * Thread Safety: This class is thread-safe and can be accessed from multiple threads simultaneously.
 * Individual codec implementations are responsible for their own thread safety.
 */
class FRuntimeCodecFactory
{
public:
	FRuntimeCodecFactory() = default;
	virtual ~FRuntimeCodecFactory() = default;

	/**
	 * Get all available codecs
	 *
	 * @return An array of all available codecs
	 */
	TArray<FBaseRuntimeCodec*> GetCodecs();

	/**
	 * Get the codec based on the audio format
	 *
	 * @param AudioFormat The format from which to get the codec
	 * 
	 * @return Array of detected codecs, or an empty array if none could be detected
	 */
	TArray<FBaseRuntimeCodec*> GetCodecs(ERuntimeAudioFormat AudioFormat);

	/**
	 * Get the codec based on the audio data (slower, but more reliable)
	 *
	 * @param AudioData The audio data from which to get the codec
	 * 
	 * @return Array of detected codecs, or an empty array if none could be detected
	 */
	TArray<FBaseRuntimeCodec*> GetCodecs(FRuntimeBulkDataBuffer<uint8>& AudioData);

	/**
	 * Get the name of the modular feature
	 * This name should be used when registering the codec as a modular feature
	 *
	 * @return The name of the modular feature
	 */
	static FName GetModularFeatureName()
	{
		static FName CodecFeatureName = FName(TEXT("RuntimeAudioImporterCodec"));
		return CodecFeatureName;
	}
};
