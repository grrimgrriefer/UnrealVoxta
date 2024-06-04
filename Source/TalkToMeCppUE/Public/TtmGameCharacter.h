// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "TtmGameCharacter.generated.h"

UCLASS()
class TALKTOMECPPUE_API ATtmGameCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATtmGameCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Main pawn camera
	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
