// Georgy Treshchev 2024.

#include "RuntimeCodecFactory.h"
#include "BaseRuntimeCodec.h"
#include "WAV_RuntimeCodec.h"
#include "Misc/Paths.h"
#include "AudioStructs.h"

DEFINE_LOG_CATEGORY(AudioLog);

TArray<FBaseRuntimeCodec*> FRuntimeCodecFactory::GetCodecs()
{
	static FCriticalSection CachedCodecsLock;
	static TArray<TUniquePtr<FBaseRuntimeCodec>> Cached;
	TArray<FBaseRuntimeCodec*> Raw;

	{
		FScopeLock Lock(&CachedCodecsLock);
		if (Cached.Num() == 0)
		{
			Cached.Emplace(MakeUnique<FWAV_RuntimeCodec>());
		}

		for (const TUniquePtr<FBaseRuntimeCodec>& Ptr : Cached)
		{
			Raw.Add(Ptr.Get());
		}
	}
	
	return Raw;
}

TArray<FBaseRuntimeCodec*> FRuntimeCodecFactory::GetCodecs(ERuntimeAudioFormat AudioFormat)
{
	TArray<FBaseRuntimeCodec*> Codecs;
	for (FBaseRuntimeCodec* Codec : GetCodecs())
	{
		if (Codec->GetAudioFormat() == AudioFormat)
		{
			Codecs.Add(Codec);
		}
	}

	if (Codecs.Num() == 0)
	{
		UE_LOG(AudioLog, Error, TEXT("Failed to determine the audio codec for the %s format"), *UEnum::GetValueAsString(AudioFormat));
	}

	return Codecs;
}

TArray<FBaseRuntimeCodec*> FRuntimeCodecFactory::GetCodecs(FRuntimeBulkDataBuffer<uint8>& AudioData)
{
	TArray<FBaseRuntimeCodec*> Codecs;
	for (FBaseRuntimeCodec* Codec : GetCodecs())
	{
		if (Codec->CheckAndFixAudioFormat(AudioData))
		{
			Codecs.Add(Codec);
		}
	}

	if (Codecs.Num() == 0)
	{
		UE_LOG(AudioLog, Error, TEXT("Failed to determine the audio codec based on the audio data of size %lld bytes"), static_cast<int64>(AudioData.GetView().Num()));
	}

	return Codecs;
}