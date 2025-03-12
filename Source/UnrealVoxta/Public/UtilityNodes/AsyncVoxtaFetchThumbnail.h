// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncVoxtaFetchThumbnail.generated.h"

class UVoxtaClient;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FVoxtaThumbnailFetched, bool, ThumbnailAvailable, const UTexture2DDynamic*, Texture, int, Width, int, Height);

UCLASS(BlueprintType, Category = "Voxta", meta = (ExposedAsyncProxy = AsyncAction))
class UNREALVOXTA_API UAsyncVoxtaFetchThumbnail : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FVoxtaThumbnailFetched ThumbnailFetched;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "worldContextObject"), Category = "Voxta")
	static UAsyncVoxtaFetchThumbnail* AsyncVoxtaFetchThumbnail(const UObject* worldContextObject, const FGuid& baseCharacterId);

	virtual void Activate() override;

private:
	UFUNCTION()
	void OnThumbnailFetched(const UTexture2DDynamic* texture, const FIntVector2& textureSize);

	UFUNCTION()
	void OnCharacterHasNoThumbnail();

	UFUNCTION()
	void OnFetchFailed();

	const UObject* m_worldContextObject;
	FGuid m_baseCharacterId;
	UVoxtaClient* m_voxtaClient;
	bool m_isActive;
};