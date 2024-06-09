// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TestGameCharacter.h"

ATestGameCharacter::ATestGameCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	m_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	m_voxtaClient = CreateDefaultSubobject<UVoxtaClient>(TEXT("VoxtaClient"));
}

void ATestGameCharacter::StartVoxtaClient()
{
	m_voxtaClient->StartConnection();
}

void ATestGameCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ATestGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATestGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("StartVoxta"), IE_Pressed, this, &ATestGameCharacter::StartVoxtaClient);
}