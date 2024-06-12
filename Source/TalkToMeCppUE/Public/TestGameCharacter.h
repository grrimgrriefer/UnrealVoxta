// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include <Voxta/Public/VoxtaClient.h>
#include "TestGameCharacter.generated.h"

UCLASS()
class TALKTOMECPPUE_API ATestGameCharacter : public APawn
{
	GENERATED_BODY()

public:
	ATestGameCharacter();
	void StartVoxtaClient();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	UCameraComponent* m_camera;

	UVoxtaClient* m_voxtaClient;
};
