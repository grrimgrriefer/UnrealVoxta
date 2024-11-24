// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "AudioCaptureDeviceInterface.h"
#include "Logging/LogMacros.h"
#include "VoxtaDefines.h"

#include "AudioStructs.generated.h"

#define WITH_DEBUG_SCOPE_LOCK false

DECLARE_LOG_CATEGORY_EXTERN(AudioLog, Log, All);

/** Possible audio formats (extensions) */
UENUM(BlueprintType, Category = "Runtime Audio Importer")
enum class ERuntimeAudioFormat : uint8
{
	Auto UMETA(DisplayName = "Determine format automatically"),
	Mp3 UMETA(DisplayName = "mp3"),
	Wav UMETA(DisplayName = "wav"),
	Flac UMETA(DisplayName = "flac"),
	OggVorbis UMETA(DisplayName = "ogg vorbis"),
	Bink UMETA(DisplayName = "bink"),
	Custom UMETA(DisplayName = "custom"),
	Invalid UMETA(DisplayName = "invalid", Hidden)
};

/** Possible RAW (uncompressed, PCM) audio formats */
UENUM(BlueprintType, Category = "Runtime Audio Importer")
enum class ERuntimeRAWAudioFormat : uint8
{
	Int8 UMETA(DisplayName = "Signed 8-bit integer"),
	UInt8 UMETA(DisplayName = "Unsigned 8-bit integer"),
	Int16 UMETA(DisplayName = "Signed 16-bit integer"),
	UInt16 UMETA(DisplayName = "Unsigned 16-bit integer"),
	Int32 UMETA(DisplayName = "Signed 32-bit integer"),
	UInt32 UMETA(DisplayName = "Unsigned 32-bit integer"),
	Float32 UMETA(DisplayName = "Floating point 32-bit")
};

/** A line of subtitle text and the time at which it should be displayed. This is the same as FSubtitleCue but editable in Blueprints */
USTRUCT(BlueprintType, Category = "Runtime Audio Importer")
struct FEditableSubtitleCue
{
	GENERATED_BODY()

	FEditableSubtitleCue()
		: Time(0)
	{}

	/** The text to appear in the subtitle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Audio Importer")
	FText Text;

	/** The time at which the subtitle is to be displayed, in seconds relative to the beginning of the line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Audio Importer")
	float Time;
};

/**
 * An alternative to FBulkDataBuffer with consistent data types
 */
template <typename DataType>
class FRuntimeBulkDataBuffer
{
public:
	using ViewType = TArrayView64<DataType>;

	FRuntimeBulkDataBuffer() = default;

	/**
	 * Reserve (pre-allocate) memory for the buffer
	 * This function can only be called if there's no data allocated
	 *
	 * @param NewCapacity New capacity to reserve
	 * @return True if the memory was successfully reserved, false otherwise
	 */
	bool Reserve(int64 NewCapacity)
	{
		// Reserve function can only be called if there's no data allocated
		if (View.GetData() != nullptr || NewCapacity <= 0)
		{
			UE_LOG(AudioLog, Error, TEXT("Reserve function can't be called if there's data allocated or NewCapacity is <= 0 (current capacity: %lld, new capacity: %lld)"), ReservedCapacity, NewCapacity);
			return false;
		}

		DataType* NewBuffer = static_cast<DataType*>(FMemory::Malloc(NewCapacity * sizeof(DataType)));
		if (!NewBuffer)
		{
			UE_LOG(AudioLog, Error, TEXT("Failed to allocate buffer to reserve memory (new capacity: %lld, %lld bytes)"), NewCapacity, NewCapacity * sizeof(DataType));
			return false;
		}

		ReservedCapacity = NewCapacity;
		View = ViewType(NewBuffer, 0);
		UE_LOG(AudioLog, Log, TEXT("Reserving memory for buffer (new capacity: %lld, %lld bytes)"), NewCapacity, NewCapacity * sizeof(DataType));

		return true;
	}

	void Append(const FRuntimeBulkDataBuffer<DataType>& Other)
	{
		Append(Other.GetView().GetData(), Other.GetView().Num());
	}

	void Append(FRuntimeBulkDataBuffer<DataType>&& Other)
	{
		Append(Other.GetView().GetData(), Other.GetView().Num());
		Other.Empty();
	}

	/**
	 * Append data to the buffer from the given buffer
	 * Takes the reserved capacity into account
	 *
	 * @param InBuffer Buffer to append data from
	 * @param InNumberOfElements Number of elements to append
	 */
	void Append(const DataType* InBuffer, int64 InNumberOfElements)
	{
		if (InNumberOfElements <= 0)
		{
			return;
		}

		// Enough reserved capacity, just memcpy
		if (ReservedCapacity > 0 && InNumberOfElements <= ReservedCapacity)
		{
			FMemory::Memcpy(View.GetData() + View.Num(), InBuffer, InNumberOfElements * sizeof(DataType));
			View = ViewType(View.GetData(), View.Num() + InNumberOfElements);
			int64 NewReservedCapacity = ReservedCapacity - InNumberOfElements;
			NewReservedCapacity = NewReservedCapacity < 0 ? 0 : NewReservedCapacity;
			UE_LOG(AudioLog, Log, TEXT("Appending data to buffer (previous capacity: %lld, new capacity: %lld)"), ReservedCapacity, NewReservedCapacity);
			ReservedCapacity = NewReservedCapacity;
		}
		// Not enough reserved capacity or no reserved capacity, reallocate entire buffer
		else
		{
			int64 NewCapacity = View.Num() + InNumberOfElements;
			DataType* NewBuffer = static_cast<DataType*>(FMemory::Malloc(NewCapacity * sizeof(DataType)));
			if (!NewBuffer)
			{
				UE_LOG(AudioLog, Error, TEXT("Failed to allocate buffer to append data (new capacity: %lld, current capacity: %lld)"), NewCapacity, View.Num());
				return;
			}

			if (View.Num() > 0)
			{
				FMemory::Memcpy(NewBuffer, View.GetData(), View.Num() * sizeof(DataType));
			}
			FMemory::Memcpy(NewBuffer + View.Num(), InBuffer, InNumberOfElements * sizeof(DataType));

			FreeBuffer();
			View = ViewType(NewBuffer, NewCapacity);
			UE_LOG(AudioLog, Log, TEXT("Reallocating buffer to append data (new capacity: %lld)"), NewCapacity);
		}
	}

	FRuntimeBulkDataBuffer(const FRuntimeBulkDataBuffer& Other)
	{
		*this = Other;
	}

	FRuntimeBulkDataBuffer(FRuntimeBulkDataBuffer&& Other) noexcept
	{
		View = MoveTemp(Other.View);
		Other.View = ViewType();
		ReservedCapacity = Other.ReservedCapacity;
		Other.ReservedCapacity = 0;
	}

	FRuntimeBulkDataBuffer(DataType* InBuffer, int64 InNumberOfElements)
		: View(InBuffer, InNumberOfElements)
	{}

	template <typename Allocator>
	explicit FRuntimeBulkDataBuffer(const TArray<DataType, Allocator>& Other)
	{
		const int64 BulkDataSize = Other.Num();

		DataType* BulkData = static_cast<DataType*>(FMemory::Malloc(BulkDataSize * sizeof(DataType)));
		if (!BulkData)
		{
			return;
		}

		FMemory::Memcpy(BulkData, Other.GetData(), BulkDataSize * sizeof(DataType));
		View = ViewType(BulkData, BulkDataSize);
		ReservedCapacity = 0;
	}

	~FRuntimeBulkDataBuffer()
	{
		FreeBuffer();
	}

	FRuntimeBulkDataBuffer& operator=(const FRuntimeBulkDataBuffer& Other)
	{
		if (this != &Other)
		{
			FreeBuffer();

			const int64 BufferSize = Other.View.Num() + Other.ReservedCapacity;

			DataType* BufferCopy = static_cast<DataType*>(FMemory::Malloc(BufferSize * sizeof(DataType)));
			FMemory::Memcpy(BufferCopy, Other.View.GetData(), BufferSize * sizeof(DataType));

			View = ViewType(BufferCopy, BufferSize);
			ReservedCapacity = Other.ReservedCapacity;
		}

		return *this;
	}

	FRuntimeBulkDataBuffer& operator=(FRuntimeBulkDataBuffer&& Other) noexcept
	{
		if (this != &Other)
		{
			FreeBuffer();
			View = MoveTemp(Other.View);
			Other.View = ViewType();
			ReservedCapacity = Other.ReservedCapacity;
			Other.ReservedCapacity = 0;
		}

		return *this;
	}

	void Empty()
	{
		FreeBuffer();
		View = ViewType();
	}

	void Reset(DataType* InBuffer, int64 InNumberOfElements)
	{
		FreeBuffer();
		View = ViewType(InBuffer, InNumberOfElements);
	}

	const ViewType& GetView() const
	{
		return View;
	}

protected:
	void FreeBuffer()
	{
		if (View.GetData() != nullptr)
		{
			FMemory::Free(View.GetData());
			View = ViewType();
			ReservedCapacity = 0;
		}
	}

	ViewType View;
	int64 ReservedCapacity = 0;
};

/** Encoded audio information */
struct FEncodedAudioStruct
{
	FEncodedAudioStruct()
		: AudioFormat(ERuntimeAudioFormat::Invalid)
	{}

	template <typename Allocator>
	FEncodedAudioStruct(const TArray<uint8, Allocator>& AudioDataArray, ERuntimeAudioFormat AudioFormat)
		: AudioData(AudioDataArray)
		, AudioFormat(AudioFormat)
	{}

	FEncodedAudioStruct(FRuntimeBulkDataBuffer<uint8> AudioDataBulk, ERuntimeAudioFormat AudioFormat)
		: AudioData(MoveTemp(AudioDataBulk))
		, AudioFormat(AudioFormat)
	{}

	/**
	 * Converts Encoded Audio Struct to a readable format
	 *
	 * @return String representation of the Encoded Audio Struct
	 */
	FString ToString() const
	{
		return FString::Printf(TEXT("Validity of audio data in memory: %s, audio data size: %lld, audio format: %s"),
			AudioData.GetView().IsValidIndex(0) ? TEXT("Valid") : TEXT("Invalid"), static_cast<int64>(AudioData.GetView().Num()),
			*UEnum::GetValueAsName(AudioFormat).ToString());
	}

	/** Audio data */
	FRuntimeBulkDataBuffer<uint8> AudioData;

	/** Format of the audio data (e.g. mp3, flac, etc) */
	ERuntimeAudioFormat AudioFormat;
};

/** Basic sound wave data */
struct FSoundWaveBasicStruct
{
	FSoundWaveBasicStruct()
		: NumOfChannels(0)
		, SampleRate(0)
		, Duration(0)
		, AudioFormat(ERuntimeAudioFormat::Invalid)
	{}

	/** Number of channels */
	uint32 NumOfChannels;

	/** Sample rate (samples per second, sampling frequency) */
	uint32 SampleRate;

	/** Sound wave duration, sec */
	float Duration;

	/** Audio format if the original audio data was encoded */
	ERuntimeAudioFormat AudioFormat;

	/**
	 * Whether the sound wave data appear to be valid or not
	 */
	bool IsValid() const
	{
		return NumOfChannels > 0 && Duration > 0;
	}

	/**
	 * Converts the basic sound wave struct to a readable format
	 *
	 * @return String representation of the basic sound wave struct
	 */
	FString ToString() const
	{
		return FString::Printf(TEXT("Number of channels: %d, sample rate: %d, duration: %f"), NumOfChannels, SampleRate, Duration);
	}
};

/** PCM data buffer structure */
struct FPCMStruct
{
	FPCMStruct()
		: PCMNumOfFrames(0)
	{}

	/**
	 * Whether the PCM data appear to be valid or not
	 */
	bool IsValid() const
	{
		return PCMData.GetView().GetData() && PCMNumOfFrames > 0 && PCMData.GetView().Num() > 0;
	}

	/**
	 * Converts PCM struct to a readable format
	 *
	 * @return String representation of the PCM Struct
	 */
	FString ToString() const
	{
		return FString::Printf(TEXT("Validity of PCM data in memory: %s, number of PCM frames: %d, PCM data size: %lld"),
			PCMData.GetView().IsValidIndex(0) ? TEXT("Valid") : TEXT("Invalid"), PCMNumOfFrames, static_cast<int64>(PCMData.GetView().Num()));
	}

	/** 32-bit float PCM data */
	FRuntimeBulkDataBuffer<float> PCMData;

	/** Number of PCM frames */
	uint32 PCMNumOfFrames;
};

/** Decoded audio information */
struct FDecodedAudioStruct
{
	/**
	 * Whether the decoded audio data appear to be valid or not
	 */
	bool IsValid() const
	{
		return SoundWaveBasicInfo.IsValid() && PCMInfo.IsValid();
	}

	/**
	 * Converts Decoded Audio Struct to a readable format
	 *
	 * @return String representation of the Decoded Audio Struct
	 */
	FString ToString() const
	{
		return FString::Printf(TEXT("SoundWave Basic Info:\n%s\n\nPCM Info:\n%s"), *SoundWaveBasicInfo.ToString(), *PCMInfo.ToString());
	}

	/** SoundWave basic info (e.g. duration, number of channels, etc) */
	FSoundWaveBasicStruct SoundWaveBasicInfo;

	/** PCM data buffer */
	FPCMStruct PCMInfo;
};

/** Platform audio input device info */
USTRUCT(BlueprintType, Category = "Runtime Audio Importer")
struct FRuntimeAudioInputDeviceInfo
{
	GENERATED_BODY()

	FRuntimeAudioInputDeviceInfo()
		: DeviceName(EMPTY_STRING)
		, DeviceId(EMPTY_STRING)
		, InputChannels(0)
		, PreferredSampleRate(0)
		, bSupportsHardwareAEC(true)
	{}

	FRuntimeAudioInputDeviceInfo(const Audio::FCaptureDeviceInfo& DeviceInfo)
		: DeviceName(DeviceInfo.DeviceName)
		, InputChannels(DeviceInfo.InputChannels)
		, PreferredSampleRate(DeviceInfo.PreferredSampleRate)
		, bSupportsHardwareAEC(DeviceInfo.bSupportsHardwareAEC)
	{}

	/** The name of the audio device */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Audio Importer")
	FString DeviceName;

	/** ID of the device */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Audio Importer")
	FString DeviceId;

	/** The number of channels supported by the audio device */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Audio Importer")
	int32 InputChannels;

	/** The preferred sample rate of the audio device */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Audio Importer")
	int32 PreferredSampleRate;

	/** Whether or not the device supports Acoustic Echo Canceling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime Audio Importer")
	bool bSupportsHardwareAEC;
};

/** Audio header information */
USTRUCT(BlueprintType, Category = "Runtime Audio Importer")
struct FRuntimeAudioHeaderInfo
{
	GENERATED_BODY()

	FRuntimeAudioHeaderInfo()
		: Duration(0.f)
		, NumOfChannels(0)
		, SampleRate(0)
		, PCMDataSize(0)
		, AudioFormat(ERuntimeAudioFormat::Invalid)
	{}

	/**
	 * Converts Audio Header Info to a readable format
	 *
	 * @return String representation of the Encoded Audio Struct
	 */
	FString ToString() const
	{
		return FString::Printf(TEXT("Duration: %f, number of channels: %d, sample rate: %d, PCM data size: %lld, audio format: %s"),
			Duration, NumOfChannels, SampleRate, PCMDataSize, *UEnum::GetValueAsName(AudioFormat).ToString());
	}

	/** Audio duration, sec */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Audio Importer")
	float Duration;

	/** Number of channels */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Audio Importer")
	int32 NumOfChannels;

	/** Sample rate (samples per second, sampling frequency) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Audio Importer")
	int32 SampleRate;

	/** PCM data size in 32-bit float format */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "PCM Data Size", Category = "Runtime Audio Importer")
	int64 PCMDataSize;

	/** Format of the source audio data (e.g. mp3, flac, etc) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Audio Importer")
	ERuntimeAudioFormat AudioFormat;
};

// This might be useful for scope lock debugging
#if WITH_DEBUG_SCOPE_LOCK
class FRAIScopeLock : public FScopeLock
{
public:
	~FRAIScopeLock()
	{
		UE_LOG(LogRuntimeAudioImporter, Verbose, TEXT("Debug scope lock destroyed"));
	}

	FRAIScopeLock(FCriticalSection* InCriticalSection)
		: FScopeLock(InCriticalSection)
	{
		UE_LOG(LogRuntimeAudioImporter, Verbose, TEXT("Debug scope lock created"));
	}
};
#else
using FRAIScopeLock = FScopeLock;
#endif
