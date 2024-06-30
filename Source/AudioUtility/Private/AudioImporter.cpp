// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AudioImporter.h"
#include "RuntimeCodecFactory.h"

void AudioImporter::ImportAudioFromBuffer(TArray64<uint8> AudioData)
{
	if (IsInGameThread())
	{
		AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [WeakThis = MakeWeakObjectPtr(this), AudioData = MoveTemp(AudioData)] () mutable
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ImportAudioFromBuffer(MoveTemp(AudioData));
			}
			else
			{
				UE_LOG(LogCore, Error, TEXT("Failed to import audio from buffer because the RuntimeAudioImporterLibrary object has been destroyed"));
			}
		});
		return;
	}

	FEncodedAudioStruct EncodedAudioInfo(AudioData, ERuntimeAudioFormat::Wav);

	FDecodedAudioStruct DecodedAudioInfo;
	if (!DecodeAudioData(MoveTemp(EncodedAudioInfo), DecodedAudioInfo))
	{
		UE_LOG(LogCore, Error, TEXT("Failed to decode audiodata: FailedToReadAudioDataArray"));
		return;
	}

	ImportAudioFromDecodedInfo(MoveTemp(DecodedAudioInfo));
}

bool AudioImporter::DecodeAudioData(FEncodedAudioStruct&& EncodedAudioInfo, FDecodedAudioStruct& DecodedAudioInfo)
{
	FRuntimeCodecFactory CodecFactory;
	TArray<FBaseRuntimeCodec*> RuntimeCodecs = [&EncodedAudioInfo, &CodecFactory] ()
		{
			if (EncodedAudioInfo.AudioFormat == ERuntimeAudioFormat::Auto)
			{
				return CodecFactory.GetCodecs(EncodedAudioInfo.AudioData);
			}
			return CodecFactory.GetCodecs(EncodedAudioInfo.AudioFormat);
		}();

		for (FBaseRuntimeCodec* RuntimeCodec : RuntimeCodecs)
		{
			EncodedAudioInfo.AudioFormat = RuntimeCodec->GetAudioFormat();
			if (!RuntimeCodec->Decode(MoveTemp(EncodedAudioInfo), DecodedAudioInfo))
			{
				UE_LOG(LogCore, Error, TEXT("Something went wrong while decoding '%s' audio data"), *UEnum::GetValueAsString(EncodedAudioInfo.AudioFormat));
				continue;
			}
			return true;
		}

		UE_LOG(LogCore, Error, TEXT("Failed to decode the audio data because the codec for the format '%s' was not found"), *UEnum::GetValueAsString(EncodedAudioInfo.AudioFormat));
		return false;
}

void AudioImporter::ImportAudioFromDecodedInfo(FDecodedAudioStruct&& DecodedAudioInfo)
{
	// Making sure we are in the game thread
	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, [WeakThis = MakeWeakObjectPtr(this), DecodedAudioInfo = MoveTemp(DecodedAudioInfo)] () mutable
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ImportAudioFromDecodedInfo(MoveTemp(DecodedAudioInfo));
			}
			else
			{
				UE_LOG(LogCore, Error, TEXT("Unable to import audio from decoded info '%s' because the RuntimeAudioImporterLibrary object has been destroyed"), *DecodedAudioInfo.ToString());
			}
		});
		return;
	}

	UImportedSoundWave* ImportedSoundWave = NewObject<UImportedSoundWave>();
	if (!ImportedSoundWave)
	{
		UE_LOG(LogCore, Error, TEXT("Something went wrong while creating the imported sound wave"));
		return;
	}

	ImportedSoundWave->AddToRoot();

	ImportedSoundWave->PopulateAudioDataFromDecodedInfo(MoveTemp(DecodedAudioInfo));

	UE_LOG(LogCore, Log, TEXT("The audio data was successfully imported"));

	ImportedSoundWave->RemoveFromRoot();
}