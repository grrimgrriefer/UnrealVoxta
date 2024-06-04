// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TtmGameCharacter.h"

// Sets default values
ATtmGameCharacter::ATtmGameCharacter()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
}

// Called when the game starts or when spawned
void ATtmGameCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATtmGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ATtmGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}