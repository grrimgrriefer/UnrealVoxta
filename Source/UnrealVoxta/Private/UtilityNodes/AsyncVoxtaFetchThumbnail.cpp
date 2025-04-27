// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#include "AsyncVoxtaFetchThumbnail.h"
#include "VoxtaClient.h"
#include "Kismet/GameplayStatics.h"

UAsyncVoxtaFetchThumbnail* UAsyncVoxtaFetchThumbnail::AsyncVoxtaFetchThumbnail(const UObject* worldContextObject, const FGuid& baseCharacterId)
{
	UAsyncVoxtaFetchThumbnail* node = NewObject<UAsyncVoxtaFetchThumbnail>();
	node->WORLD_CONTEXT = worldContextObject;
	node->m_baseCharacterId = baseCharacterId;
	node->RegisterWithGameInstance(worldContextObject);
	return node;
}

void UAsyncVoxtaFetchThumbnail::Activate()
{
	if (m_isActive)
	{
		FFrame::KismetExecutionMessage(TEXT("Thumbnail fetch is already in progress."), ELogVerbosity::Warning);
		return;
	}

	if (!WORLD_CONTEXT)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid WorldContextObject. Cannot fetch thumbnail."), ELogVerbosity::Error);
		OnThumbnailFetched(false, nullptr, FIntVector2());
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WORLD_CONTEXT);
	if (!GameInstance)
	{
		FFrame::KismetExecutionMessage(TEXT("Could not get Game Instance."), ELogVerbosity::Error);
		OnThumbnailFetched(false, nullptr, FIntVector2());
		return;
	}

	m_voxtaClient = GameInstance->GetSubsystem<UVoxtaClient>();
	if (!m_voxtaClient)
	{
		FFrame::KismetExecutionMessage(TEXT("Could not get VoxtaClient Subsystem."), ELogVerbosity::Error);
		OnThumbnailFetched(false, nullptr, FIntVector2());
		return;
	}

	m_isActive = true;

	FDownloadedTextureDelegateNative fetchedTexture = FDownloadedTextureDelegateNative::CreateWeakLambda(this,
		[this] (bool bSuccess, const UTexture2DDynamic* Texture, FIntVector2 Size)
		{
				OnThumbnailFetched(bSuccess, Texture, Size);
		}
	);
	m_voxtaClient->TryFetchAndCacheCharacterThumbnail(m_baseCharacterId, fetchedTexture);
}

void UAsyncVoxtaFetchThumbnail::OnThumbnailFetched(bool success, const UTexture2DDynamic* texture, const FIntVector2& textureSize)
{
	ThumbnailFetched.Broadcast(success, texture, textureSize.X, textureSize.Y);
	m_isActive = false;
	SetReadyToDestroy();
}