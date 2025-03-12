// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#include "UtilityNodes/AsyncVoxtaFetchThumbnail.h"
#include "VoxtaClient.h"
#include "Kismet/GameplayStatics.h"

UAsyncVoxtaFetchThumbnail* UAsyncVoxtaFetchThumbnail::AsyncVoxtaFetchThumbnail(const UObject* worldContextObject, const FGuid& baseCharacterId)
{
	UAsyncVoxtaFetchThumbnail* node = NewObject<UAsyncVoxtaFetchThumbnail>();
	node->m_worldContextObject = worldContextObject;
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

	if (!m_worldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid WorldContextObject. Cannot fetch thumbnail."), ELogVerbosity::Error);
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(m_worldContextObject);
	if (!GameInstance)
	{
		FFrame::KismetExecutionMessage(TEXT("Could not get Game Instance."), ELogVerbosity::Error);
		SetReadyToDestroy();
		return;
	}

	m_voxtaClient = GameInstance->GetSubsystem<UVoxtaClient>();
	if (!m_voxtaClient)
	{
		FFrame::KismetExecutionMessage(TEXT("Could not get VoxtaClient Subsystem."), ELogVerbosity::Error);
		SetReadyToDestroy();
		return;
	}

	m_isActive = true;

	FDownloadedTextureDelegateNative fetchedTexture = FDownloadedTextureDelegateNative::CreateUObject(this, &UAsyncVoxtaFetchThumbnail::OnThumbnailFetched);
	UVoxtaClient::FVoxtaCharacterHasNoThumbnailNative noTexture = UVoxtaClient::FVoxtaCharacterHasNoThumbnailNative::CreateUObject(this, &UAsyncVoxtaFetchThumbnail::OnCharacterHasNoThumbnail);
	m_voxtaClient->TryFetchAndCacheCharacterThumbnail(m_baseCharacterId, noTexture, fetchedTexture);
}

void UAsyncVoxtaFetchThumbnail::OnThumbnailFetched(const UTexture2DDynamic* texture, const FIntVector2& textureSize)
{
	if (m_isActive)
	{
		ThumbnailFetched.Broadcast(true, texture, textureSize.X, textureSize.Y);
		m_isActive = false;
		SetReadyToDestroy();
	}
}

void UAsyncVoxtaFetchThumbnail::OnCharacterHasNoThumbnail()
{
	if (m_isActive)
	{
		ThumbnailFetched.Broadcast(false, nullptr, 0, 0);
		m_isActive = false;
		SetReadyToDestroy();
	}
}

void UAsyncVoxtaFetchThumbnail::OnFetchFailed()
{
	if (m_isActive)
	{
		FFrame::KismetExecutionMessage(TEXT("Thumbnail fetch failed."), ELogVerbosity::Error);
		m_isActive = false;
		SetReadyToDestroy();
	}
}