// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TexturesCacheHandler.h"
#include "VoxtaDefines.h"
#include "HttpModule.h"
#include "Interfaces/IHttpBase.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2DDynamic.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "TextureResource.h"
#include "RenderingThread.h"
#include "Logging/StructuredLog.h"
#include "LogUtility/Public/Defines.h"

TexturesCacheHandler::TexturesCacheHandler()
{
	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName(TEXT("ImageWrapper")));
	m_imageWrappers =
	{
		imageWrapperModule.CreateImageWrapper(EImageFormat::PNG),
		imageWrapperModule.CreateImageWrapper(EImageFormat::JPEG),
		imageWrapperModule.CreateImageWrapper(EImageFormat::BMP),
	};
}

void TexturesCacheHandler::FetchTextureFromUrl(const FString& url, FDownloadedTextureDelegateNative onThumbnailFetched)
{
	if (m_texturesCache.Contains(url))
	{
		onThumbnailFetched.ExecuteIfBound(true, m_texturesCache[url].TEXTURE.Get(), m_texturesCache[url].TEXTURE_SIZE);
		return;
	}
	else if (m_pendingCallbacks.Contains(url))
	{
		m_pendingCallbacks[url].Emplace(onThumbnailFetched);
		return;
	}

	TArray<FDownloadedTextureDelegateNative> callbackArray;
	callbackArray.Add(onThumbnailFetched);
	m_pendingCallbacks.Add(url, callbackArray);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> httpRequest = FHttpModule::Get().CreateRequest();
	httpRequest->SetURL(url);
	httpRequest->SetVerb(TEXT("GET"));
	httpRequest->OnProcessRequestComplete().BindLambda([Self = TWeakPtr<TexturesCacheHandler>(AsShared()), URL = url]
	(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful)
		{
			if (TSharedPtr<TexturesCacheHandler> sharedSelf = Self.Pin())
			{
				if (bWasSuccessful && response.IsValid() && EHttpResponseCodes::IsOk(response->GetResponseCode()) &&
					response->GetContentLength() > 0 && response->GetContent().Num() > 0)
				{				
					for (auto& imageWrapper : sharedSelf->m_imageWrappers)
					{
						if (imageWrapper.IsValid() &&
							imageWrapper->SetCompressed(response->GetContent().GetData(),
							response->GetContent().Num()) &&
							imageWrapper->GetWidth() <= TNumericLimits<int32>::Max() &&
							imageWrapper->GetHeight() <= TNumericLimits<int32>::Max())
						{
							TArray64<uint8> rawData;
							const ERGBFormat inFormat = ERGBFormat::BGRA;
							if (imageWrapper->GetRaw(inFormat, 8, rawData))
							{
								int32 width = static_cast<int32>(imageWrapper->GetWidth());
								int32 height = static_cast<int32>(imageWrapper->GetHeight());
								if (UTexture2DDynamic* texture = UTexture2DDynamic::Create(width, height))
								{
									texture->SRGB = true;
									texture->UpdateResource();

									FTexture2DDynamicResource* textureResource =
										static_cast<FTexture2DDynamicResource*>(texture->GetResource());
									if (textureResource)
									{
										ENQUEUE_RENDER_COMMAND(FWriteRawDataToTexture)(
											[textureResource, RawData = MoveTemp(rawData)]
											(FRHICommandListImmediate& RHICmdList)
											{
												textureResource->WriteRawToTexture_RenderThread(RawData);
											});
									}
									sharedSelf->m_texturesCache.Emplace(URL, TextureInfo(MoveTemp(texture), width, height));
									for (auto& var : sharedSelf->m_pendingCallbacks[URL])
									{
										var.ExecuteIfBound(true, sharedSelf->m_texturesCache[URL].TEXTURE.Get(),
											sharedSelf->m_texturesCache[URL].TEXTURE_SIZE);
									}
									sharedSelf->m_pendingCallbacks.Remove(URL);
									return;
								}
							}
						}
					}
				}
				// Failed to process / fetch image
				SENSITIVE_LOG1(VoxtaLog, Warning, "Failed to fetch thumbnail from: {0}", URL);
				for (auto& var : sharedSelf->m_pendingCallbacks[URL])
				{
					var.ExecuteIfBound(false, nullptr, FIntVector2());
				}
				sharedSelf->m_pendingCallbacks.Remove(URL);
			}
			else
			{
				SENSITIVE_LOG1(VoxtaLog, Error, "Downloaded data from: {0} "
					"But the TexturesCacheHandler was destroyed?", request->GetURL());
			}			
		});

	httpRequest->ProcessRequest();
}