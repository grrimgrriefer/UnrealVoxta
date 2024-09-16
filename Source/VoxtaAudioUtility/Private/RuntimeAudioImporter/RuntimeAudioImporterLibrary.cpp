// Georgy Treshchev 2024.

#include "RuntimeAudioImporter/RuntimeAudioImporterLibrary.h"
#include "RuntimeAudioImporter/RuntimeCodecFactory.h"
#include "RuntimeAudioImporter/ImportedSoundWave.h"
#include "RuntimeAudioImporter/RAW_RuntimeCodec.h"

void URuntimeAudioImporterLibrary::ImportAudioFromBuffer(TArray64<uint8> AudioData, TFunction<void(UImportedSoundWave*)> callback)
{
	if (IsInGameThread())
	{
		AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [AudioData = MoveTemp(AudioData), Callback = callback] () mutable
		{
			ImportAudioFromBuffer(MoveTemp(AudioData), Callback);
		});
		return;
	}

	FEncodedAudioStruct EncodedAudioInfo(AudioData, ERuntimeAudioFormat::Wav);

	FDecodedAudioStruct DecodedAudioInfo;
	if (!DecodeAudioData(MoveTemp(EncodedAudioInfo), DecodedAudioInfo))
	{
		UE_LOG(AudioLog, Error, TEXT("Failed to decode audiodata: FailedToReadAudioDataArray"));
		return;
	}

	ImportAudioFromDecodedInfo(MoveTemp(DecodedAudioInfo), callback);
}

bool URuntimeAudioImporterLibrary::DecodeAudioData(FEncodedAudioStruct&& EncodedAudioInfo, FDecodedAudioStruct& DecodedAudioInfo)
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
			UE_LOG(AudioLog, Error, TEXT("Something went wrong while decoding '%s' audio data"), *UEnum::GetValueAsString(EncodedAudioInfo.AudioFormat));
			continue;
		}
		return true;
	}

	UE_LOG(AudioLog, Error, TEXT("Failed to decode the audio data because the codec for the format '%s' was not found"), *UEnum::GetValueAsString(EncodedAudioInfo.AudioFormat));
	return false;
}

bool URuntimeAudioImporterLibrary::EncodeAudioData(FDecodedAudioStruct&& DecodedAudioInfo, FEncodedAudioStruct& EncodedAudioInfo, uint8 Quality)
{
	if (EncodedAudioInfo.AudioFormat == ERuntimeAudioFormat::Auto || EncodedAudioInfo.AudioFormat == ERuntimeAudioFormat::Invalid)
	{
		UE_LOG(AudioLog, Error, TEXT("Undefined audio data format for encoding"));
		return false;
	}

	FRuntimeCodecFactory CodecFactory;
	TArray<FBaseRuntimeCodec*> RuntimeCodecs = CodecFactory.GetCodecs(EncodedAudioInfo.AudioFormat);
	for (FBaseRuntimeCodec* RuntimeCodec : RuntimeCodecs)
	{
		if (!RuntimeCodec->Encode(MoveTemp(DecodedAudioInfo), EncodedAudioInfo, Quality))
		{
			UE_LOG(AudioLog, Error, TEXT("Something went wrong while encoding '%s' audio data"), *UEnum::GetValueAsString(EncodedAudioInfo.AudioFormat));
			continue;
		}
		return true;
	}
	UE_LOG(AudioLog, Error, TEXT("Failed to encode the audio data because the codec for the format '%s' was not found"), *UEnum::GetValueAsString(EncodedAudioInfo.AudioFormat));
	return false;
}

void URuntimeAudioImporterLibrary::ImportAudioFromDecodedInfo(FDecodedAudioStruct&& DecodedAudioInfo, TFunction<void(UImportedSoundWave*)> callback)
{
	// Making sure we are in the game thread
	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, [DecodedAudioInfo = MoveTemp(DecodedAudioInfo), Callback = callback] () mutable
		{
			ImportAudioFromDecodedInfo(MoveTemp(DecodedAudioInfo), Callback);
		});
		return;
	}

	UImportedSoundWave* ImportedSoundWave = NewObject<UImportedSoundWave>();
	if (!ImportedSoundWave)
	{
		UE_LOG(AudioLog, Error, TEXT("Something went wrong while creating the imported sound wave"));
		return;
	}

	ImportedSoundWave->AddToRoot();
	ImportedSoundWave->PopulateAudioDataFromDecodedInfo(MoveTemp(DecodedAudioInfo));

	UE_LOG(AudioLog, Log, TEXT("The audio data was successfully imported"));
	callback(ImportedSoundWave);
}

bool URuntimeAudioImporterLibrary::ResampleAndMixChannelsInDecodedInfo(FDecodedAudioStruct& DecodedAudioInfo, uint32 NewSampleRate, uint32 NewNumOfChannels)
{
	if (DecodedAudioInfo.SoundWaveBasicInfo.SampleRate <= 0 || NewSampleRate <= 0)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to resample audio data because the sample rate is invalid (Current: %d, New: %d)"), DecodedAudioInfo.SoundWaveBasicInfo.SampleRate, NewSampleRate);
		return false;
	}

	if (DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels <= 0 || NewNumOfChannels <= 0)
	{
		UE_LOG(AudioLog, Error, TEXT("Unable to mix audio data because the number of channels is invalid (Current: %d, New: %d)"), DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels, NewNumOfChannels);
		return false;
	}

	if (NewSampleRate == DecodedAudioInfo.SoundWaveBasicInfo.SampleRate && NewNumOfChannels == DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels)
	{
		UE_LOG(AudioLog, Log, TEXT("No need to resample or mix audio data"));
		return true;
	}

	Audio::FAlignedFloatBuffer WaveData(DecodedAudioInfo.PCMInfo.PCMData.GetView().GetData(), DecodedAudioInfo.PCMInfo.PCMData.GetView().Num());

	// Resampling if needed
	if (NewSampleRate != DecodedAudioInfo.SoundWaveBasicInfo.SampleRate)
	{
		Audio::FAlignedFloatBuffer ResamplerOutputData;
		if (!FRAW_RuntimeCodec::ResampleRAWData(WaveData, DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels, DecodedAudioInfo.SoundWaveBasicInfo.SampleRate, NewSampleRate, ResamplerOutputData))
		{
			UE_LOG(AudioLog, Error, TEXT("Unable to resample audio data to the sound wave's sample rate. Resampling failed"));
			return false;
		}
		WaveData = MoveTemp(ResamplerOutputData);
		DecodedAudioInfo.SoundWaveBasicInfo.SampleRate = NewSampleRate;
		UE_LOG(AudioLog, Log, TEXT("Audio data has been resampled to the desired sample rate '%d'"), NewSampleRate);
	}

	// Mixing the channels if needed
	if (NewNumOfChannels != DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels)
	{
		Audio::FAlignedFloatBuffer WaveDataTemp;
		if (!FRAW_RuntimeCodec::MixChannelsRAWData(WaveData, NewSampleRate, DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels, NewNumOfChannels, WaveDataTemp))
		{
			UE_LOG(AudioLog, Error, TEXT("Unable to mix audio data to the sound wave's number of channels. Mixing failed"));
			return false;
		}
		WaveData = MoveTemp(WaveDataTemp);
		DecodedAudioInfo.SoundWaveBasicInfo.NumOfChannels = NewNumOfChannels;
		UE_LOG(AudioLog, Log, TEXT("Audio data has been mixed to the desired number of channels '%d'"), NewNumOfChannels);
	}

	DecodedAudioInfo.PCMInfo.PCMData = FRuntimeBulkDataBuffer<float>(WaveData);
	return true;
}