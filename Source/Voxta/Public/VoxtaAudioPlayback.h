// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "VoxtaClient.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "VoxtaAudioPlayback.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VOXTA_API UVoxtaAudioPlayback : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxtaAudioPlayback();

	void InitializeAudioPlayback(UVoxtaClient* voxtaClient, FStringView characterId);

private:
	FString m_characterId;
	UAudioComponent* m_audioComponent;

	FString m_hostAddress;
	FString m_hostPort;

	UFUNCTION()
	void PlaybackMessage(const FCharDataBase& sender, const FChatMessage& message);

	void DownloadFile(FStringView url);
	void OnDownloadComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful);
};
