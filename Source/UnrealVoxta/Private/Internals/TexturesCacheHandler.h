// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "VoxtaDefines.h"

class UTexture2DDynamic;
class IImageWrapper;

/**
 * TexturesCacheHandler
 * Internal class which handles asynchronous fetching, decoding, and caching of textures from URLs.
 * Downloads image data from URLs, converts it into textures, and caches them for future use.
 *
 * Note: The private API hooks into callbacks from background-threads, use care when changing the implementation.
 */
class TexturesCacheHandler : public TSharedFromThis<TexturesCacheHandler>
{
#pragma region helper classes
private:
	class TextureInfo
	{
	public:
		TextureInfo(TWeakObjectPtr<const UTexture2DDynamic> texture, int width, int height) :
			TEXTURE(texture), TEXTURE_SIZE(width, height)
		{}
		TWeakObjectPtr<const UTexture2DDynamic> TEXTURE;
		const FIntVector2 TEXTURE_SIZE;
	};
#pragma endregion

#pragma region public API
public:
	TexturesCacheHandler();
	void FetchTextureFromUrl(const FString& url, FDownloadedTextureDelegateNative onThumbnailFetched);
#pragma endregion

#pragma region data
private:
	TMap<FString, TArray<FDownloadedTextureDelegateNative>> m_pendingCallbacks;
	TMap<FString, TextureInfo> m_texturesCache;
	TArray<TSharedPtr<IImageWrapper>> m_imageWrappers;
#pragma endregion
};
