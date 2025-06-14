// Georgy Treshchev 2024.

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "HAL/UnrealMemory.h"
#include "AudioStructs.h"
#include "SampleBuffer.h"
#include "AudioResampler.h"
#include <type_traits>
#include <limits>

/**
 * RAW format codec implementation for runtime audio importing.
 * Provides utilities for transcoding, resampling, and mixing RAW audio data buffers.
 * Supports conversion between different integer and floating-point sample formats.
 */
class FRAW_RuntimeCodec
{
public:
	/**
	 * Get the minimum and maximum representable values for a given integral or floating-point type.
	 *
	 * @tparam IntegralType The type for which to get the min and max values (e.g., int16, uint8, float).
	 *
	 * @return A tuple where Key is the minimum and Value is the maximum value for the type.
	 */
	template <typename IntegralType>
	static TTuple<long long, long long> GetRawMinAndMaxValues()
	{
		// Signed 8-bit integer
		if (std::is_same<IntegralType, int8>::value)
		{
			return TTuple<long long, long long>((std::numeric_limits<int8>::min)(), (std::numeric_limits<int8>::max)());
		}

		// Unsigned 8-bit integer
		if (std::is_same<IntegralType, uint8>::value)
		{
			return TTuple<long long, long long>((std::numeric_limits<uint8>::min)(), (std::numeric_limits<uint8>::max)());
		}

		// Signed 16-bit integer
		if (std::is_same<IntegralType, int16>::value)
		{
			return TTuple<long long, long long>((std::numeric_limits<int16>::min)(), (std::numeric_limits<int16>::max)());
		}

		// Unsigned 16-bit integer
		if (std::is_same<IntegralType, uint16>::value)
		{
			return TTuple<long long, long long>((std::numeric_limits<uint16>::min)(), (std::numeric_limits<uint16>::max)());
		}

		// Signed 32-bit integer
		if (std::is_same<IntegralType, int32>::value)
		{
			return TTuple<long long, long long>((std::numeric_limits<int32>::min)(), (std::numeric_limits<int32>::max)());
		}

		// Unsigned 32-bit integer
		if (std::is_same<IntegralType, uint32>::value)
		{
			return TTuple<long long, long long>((std::numeric_limits<uint32>::min)(), (std::numeric_limits<uint32>::max)());
		}

		// Floating point 32-bit
		if (std::is_same<IntegralType, float>::value)
		{
			return TTuple<long long, long long>(-1, 1);
		}

		ensureMsgf(false, TEXT("Unsupported RAW format"));
		return TTuple<long long, long long>(0, 0);
	}

	/**
	 * Transcode a buffer of RAW audio data from one sample format to another.
	 *
	 * @tparam IntegralTypeFrom The source sample type.
	 * @tparam IntegralTypeTo The destination sample type.
	 * @param RAWData_From Input buffer containing audio data in the source format.
	 * @param RAWData_To Output buffer to receive audio data in the destination format.
	 */
	template <typename IntegralTypeFrom, typename IntegralTypeTo>
	static void TranscodeRAWData(const TArray64<uint8>& RAWData_From, TArray64<uint8>& RAWData_To)
	{
		const IntegralTypeFrom* DataFrom = reinterpret_cast<const IntegralTypeFrom*>(RAWData_From.GetData());
		const int64 RawDataSize = RAWData_From.Num() / sizeof(IntegralTypeFrom);

		RAWData_To.SetNumUninitialized(RawDataSize * sizeof(IntegralTypeTo));
		IntegralTypeTo* DataTo = reinterpret_cast<IntegralTypeTo*>(RAWData_To.GetData());
		TranscodeRAWData<IntegralTypeFrom, IntegralTypeTo>(DataFrom, RawDataSize, DataTo);
	}

	/**
	 * Transcode a buffer of RAW audio data from one sample format to another (pointer version).
	 *
	 * @tparam IntegralTypeFrom The source sample type.
	 * @tparam IntegralTypeTo The destination sample type.
	 * @param RAWDataFrom Pointer to the input audio data in the source format.
	 * @param NumOfSamples Number of samples in the input buffer.
	 * @param RAWDataTo Pointer reference to receive the transcoded audio data in the destination format.
	 */
	template <typename IntegralTypeFrom, typename IntegralTypeTo>
	static void TranscodeRAWData(const IntegralTypeFrom* RAWDataFrom, int64 NumOfSamples, IntegralTypeTo*& RAWDataTo)
	{
		/** Creating an empty PCM buffer */
		RAWDataTo = static_cast<IntegralTypeTo*>(FMemory::Malloc(NumOfSamples * sizeof(IntegralTypeTo)));

		const TTuple<long long, long long> MinAndMaxValuesFrom{ GetRawMinAndMaxValues<IntegralTypeFrom>() };
		const TTuple<long long, long long> MinAndMaxValuesTo{ GetRawMinAndMaxValues<IntegralTypeTo>() };

		/** Iterating through the RAW Data to transcode values using a divisor */
		for (int64 SampleIndex = 0; SampleIndex < NumOfSamples; ++SampleIndex)
		{
			RAWDataTo[SampleIndex] = static_cast<IntegralTypeTo>(FMath::GetMappedRangeValueClamped(FVector2D(MinAndMaxValuesFrom.Key, MinAndMaxValuesFrom.Value), FVector2D(MinAndMaxValuesTo.Key, MinAndMaxValuesTo.Value), RAWDataFrom[SampleIndex]));
		}

		UE_LOG(AudioLog, Log, TEXT("Transcoding RAW data of size '%llu' (min: %lld, max: %lld) to size '%llu' (min: %lld, max: %lld)"),
			   static_cast<uint64>(sizeof(IntegralTypeFrom)), MinAndMaxValuesFrom.Key, MinAndMaxValuesFrom.Value, static_cast<uint64>(sizeof(IntegralTypeTo)), MinAndMaxValuesTo.Key, MinAndMaxValuesTo.Value);
	}

	/**
	 * Resample RAW audio data to a different sample rate.
	 *
	 * @param RAWData Input/output buffer containing audio data to be resampled.
	 * @param NumOfChannels Number of channels in the audio data.
	 * @param SourceSampleRate The original sample rate of the audio data.
	 * @param DestinationSampleRate The desired sample rate after resampling.
	 * @param ResampledRAWData Output buffer to receive the resampled audio data.
	 *
	 * @return True if resampling was successful, false otherwise.
	 */
	static bool ResampleRAWData(Audio::FAlignedFloatBuffer& RAWData, uint32 NumOfChannels, uint32 SourceSampleRate, uint32 DestinationSampleRate, Audio::FAlignedFloatBuffer& ResampledRAWData)
	{
		if (NumOfChannels <= 0)
		{
			UE_LOG(AudioLog, Error, TEXT("Unable to resample audio data because the number of channels is invalid (%d)"), NumOfChannels);
			return false;
		}
		if (SourceSampleRate <= 0)
		{
			UE_LOG(AudioLog, Error, TEXT("Unable to resample audio data because the source sample rate is invalid (%d)"), SourceSampleRate);
			return false;
		}
		if (DestinationSampleRate <= 0)
		{
			UE_LOG(AudioLog, Error, TEXT("Unable to resample audio data because the destination sample rate is invalid (%d)"), DestinationSampleRate);
			return false;
		}

		// No need to resample if the sample rates are the same
		if (SourceSampleRate == DestinationSampleRate)
		{
			ResampledRAWData = RAWData;
			return true;
		}

		const Audio::FResamplingParameters ResampleParameters = {
			Audio::EResamplingMethod::BestSinc,
			static_cast<int32>(NumOfChannels),
			static_cast<float>(SourceSampleRate),
			static_cast<float>(DestinationSampleRate),
			RAWData
		};

		ResampledRAWData.Reset();
		ResampledRAWData.AddUninitialized(Audio::GetOutputBufferSize(ResampleParameters));
		Audio::FResamplerResults ResampleResults;
		ResampleResults.OutBuffer = &ResampledRAWData;

		if (!Audio::Resample(ResampleParameters, ResampleResults))
		{
			UE_LOG(AudioLog, Error, TEXT("Unable to resample audio data from %d to %d"), SourceSampleRate, DestinationSampleRate);
			return false;
		}

		return true;
	}

	/**
	 * Mix RAW audio data to a different number of channels.
	 *
	 * @param RAWData Input/output buffer containing audio data to be remixed.
	 * @param SampleRate Sample rate of the audio data.
	 * @param SourceNumOfChannels The original number of channels in the audio data.
	 * @param DestinationNumOfChannels The desired number of channels after mixing.
	 * @param RemixedRAWData Output buffer to receive the remixed audio data.
	 *
	 * @return True if mixing was successful, false otherwise.
	 */
	static bool MixChannelsRAWData(Audio::FAlignedFloatBuffer& RAWData, int32 SampleRate, int32 SourceNumOfChannels, int32 DestinationNumOfChannels, Audio::FAlignedFloatBuffer& RemixedRAWData)
	{
		if (SampleRate <= 0)
		{
			UE_LOG(AudioLog, Error, TEXT("Unable to mix audio data because the sample rate is invalid (%d)"), SampleRate);
			return false;
		}
		if (SourceNumOfChannels <= 0)
		{
			UE_LOG(AudioLog, Error, TEXT("Unable to mix audio data because the source number of channels is invalid (%d)"), SourceNumOfChannels);
			return false;
		}
		if (DestinationNumOfChannels <= 0)
		{
			UE_LOG(AudioLog, Error, TEXT("Unable to mix audio data because the destination number of channels is invalid (%d)"), DestinationNumOfChannels);
			return false;
		}

		// No need to mix if the number of channels are the same
		if (SourceNumOfChannels == DestinationNumOfChannels)
		{
			RemixedRAWData = MoveTemp(RAWData);
			return true;
		}

		Audio::TSampleBuffer<float> PCMSampleBuffer(RAWData, SourceNumOfChannels, SampleRate);
		{
			PCMSampleBuffer.MixBufferToChannels(DestinationNumOfChannels);
		}
		RemixedRAWData = Audio::FAlignedFloatBuffer(PCMSampleBuffer.GetData(), PCMSampleBuffer.GetNumSamples());
		return true;
	}
};
