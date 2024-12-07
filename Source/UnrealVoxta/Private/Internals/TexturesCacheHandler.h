// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"

class UTexture2DDynamic;
class IImageWrapper;

DECLARE_DELEGATE_OneParam(FDownloadedTextureDelegateNative, UTexture2DDynamic*);

/**
 * TexturesCacheHandler
 * Internal class which encapsulates all datahandling for a single AI character voiceline.
 * Downloads the data from the VoxtaServer REST api, converting it into a SoundWave, and generating lipsync data.
 *
 * Note: The private API hooks into callbacks from background-threads, use care when changing the implementation.
 */
class TexturesCacheHandler : public TSharedFromThis<TexturesCacheHandler>
{
#pragma region public API
public:
	TexturesCacheHandler();
	void FetchTextureFromUrl(const FString& url, FDownloadedTextureDelegateNative onThumbnailFetched);
#pragma endregion

#pragma region data
private:
	TMap<FString, TArray<FDownloadedTextureDelegateNative>> m_pendingCallbacks;
	TMap<FString, UTexture2DDynamic*> m_texturesCache;
	TArray<TSharedPtr<IImageWrapper>> m_imageWrappers;
#pragma endregion
};
