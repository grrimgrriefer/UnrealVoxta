// Georgy Treshchev 2024.

#include "RuntimeAudioImporter/RuntimeCodecFactory.h"
#include "RuntimeAudioImporter/BaseRuntimeCodec.h"
#include "RuntimeAudioImporter/WAV_RuntimeCodec.h"
#include "Misc/Paths.h"
#include "RuntimeAudioImporter/AudioStructs.h"

DEFINE_LOG_CATEGORY(AudioLog);

TArray<FBaseRuntimeCodec*> FRuntimeCodecFactory::GetCodecs()
{
	TArray<FBaseRuntimeCodec*> AvailableCodecs = { new FWAV_RuntimeCodec() };
	return AvailableCodecs;
}

TArray<FBaseRuntimeCodec*> FRuntimeCodecFactory::GetCodecs(const FString& FilePath)
{
	TArray<FBaseRuntimeCodec*> Codecs;
	const FString Extension = FPaths::GetExtension(FilePath, false);

	for (FBaseRuntimeCodec* Codec : GetCodecs())
	{
		if (Codec->IsExtensionSupported(Extension))
		{
			Codecs.Add(Codec);
		}
	}

	if (Codecs.Num() == 0)
	{
		UE_LOG(AudioLog, Warning, TEXT("Failed to determine the audio codec for '%s' using its file name"), *FilePath);
	}

	return Codecs;
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

TArray<FBaseRuntimeCodec*> FRuntimeCodecFactory::GetCodecs(const FRuntimeBulkDataBuffer<uint8>& AudioData)
{
	TArray<FBaseRuntimeCodec*> Codecs;
	for (FBaseRuntimeCodec* Codec : GetCodecs())
	{
		if (Codec->CheckAudioFormat(AudioData))
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