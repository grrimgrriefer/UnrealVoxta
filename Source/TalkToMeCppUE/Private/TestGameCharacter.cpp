// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TestGameCharacter.h"
#include "Kismet/GameplayStatics.h"

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
	SetupHud();
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

void ATestGameCharacter::SetupHud()
{
	m_hud = UGameplayStatics::GetPlayerController(this, 0)->GetHUD<ATalkToMeCppUeHUD>();

	m_voxtaClient->OnVoxtaClientStateChangedDelegate.AddUniqueDynamic(m_hud, &ATalkToMeCppUeHUD::VoxtaClientStateChanged);
	m_voxtaClient->OnVoxtaClientCharacterLoadedDelegate.AddUniqueDynamic(m_hud, &ATalkToMeCppUeHUD::VoxtaClientCharacterLoaded);
	m_voxtaClient->OnVoxtaClientChatMessageAdded.AddUniqueDynamic(m_hud, &ATalkToMeCppUeHUD::RegisterTextMessage);
	m_voxtaClient->OnVoxtaClientChatMessageRemoved.AddUniqueDynamic(m_hud, &ATalkToMeCppUeHUD::RemoveTextMessage);

	m_hud->OnCharButtonClickedDelegate.AddUniqueDynamic(m_voxtaClient, &UVoxtaClient::LoadCharacter);
	m_hud->OnUserInputFieldSubmittedDelegate.AddUniqueDynamic(m_voxtaClient, &UVoxtaClient::SendUserInput);
}