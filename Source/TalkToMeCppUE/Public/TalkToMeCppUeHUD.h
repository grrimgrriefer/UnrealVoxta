// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TalkToMeCppUeWidget.h"
#include "VoxtaClient.h"
#include "VoxtaData/Public/CharData.h"
#include "VoxtaData/Public/ChatMessage.h"
#include "Containers/StringFwd.h"
#include "Voxta/Private/VoxtaLogUtility.h"
#include "Logging/StructuredLog.h"
#include "TalkToMeCppUeHUD.generated.h"

UCLASS()
class TALKTOMECPPUE_API ATalkToMeCppUeHUD : public AHUD
{
	GENERATED_BODY()

public:
	ATalkToMeCppUeHUD();

	virtual void BeginPlay() override;

	UPROPERTY()
	FCharButtonClickedSignature OnCharButtonClickedDelegate;

	UFUNCTION()
	void VoxtaClientStateChanged(VoxtaClientState newState);
	UFUNCTION()
	void VoxtaClientCharacterLoaded(const FCharData& charData);
	UFUNCTION()
	void RegisterTextMessage(const FCharData& sender, const FChatMessage& message);

private:
	class UClass* m_hudWidgetClass;
	class UTalkToMeCppUeWidget* m_hudWidget;

	UFUNCTION()
	void OnCharButtonClicked(FString charID);
};
