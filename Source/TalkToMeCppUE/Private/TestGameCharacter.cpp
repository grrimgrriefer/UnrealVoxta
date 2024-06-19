// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "TestGameCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"

ATestGameCharacter::ATestGameCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	m_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	m_voxtaClient = CreateDefaultSubobject<UVoxtaClient>(TEXT("VoxtaClient"));
	m_hud = nullptr;
}

void ATestGameCharacter::StartVoxtaClient()
{
	m_voxtaClient->StartConnection();
}

void ATestGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_hud = UGameplayStatics::GetPlayerController(this, 0)->GetHUD<ATalkToMeCppUeHUD>();
	if (!TryConnectToHud())
	{
		UE_LOGFMT(LogCore, Error, "Failed to connect the ATestGameCharacter to the ATalkToMeCppUeHUD.");
	}
}

void ATestGameCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!TryDisconnectToHud() && EndPlayReason != EEndPlayReason::Quit)
	{
		UE_LOGFMT(LogCore, Error, "Failed to clean up the bindings between the ATestGameCharacter and the ATalkToMeCppUeHUD.");
	}
	m_hud = nullptr;
}

void ATestGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("StartVoxta"), IE_Pressed, this, &ATestGameCharacter::StartVoxtaClient);
}

bool ATestGameCharacter::TryConnectToHud()
{
	if (m_voxtaClient && m_hud)
	{
		m_voxtaClient->OnVoxtaClientStateChangedDelegate.AddUniqueDynamic(m_hud, &ATalkToMeCppUeHUD::VoxtaClientStateChanged);
		m_voxtaClient->OnVoxtaClientCharacterLoadedDelegate.AddUniqueDynamic(m_hud, &ATalkToMeCppUeHUD::VoxtaClientCharacterLoaded);
		m_voxtaClient->OnVoxtaClientChatMessageAdded.AddUniqueDynamic(m_hud, &ATalkToMeCppUeHUD::RegisterTextMessage);
		m_voxtaClient->OnVoxtaClientChatMessageRemoved.AddUniqueDynamic(m_hud, &ATalkToMeCppUeHUD::RemoveTextMessage);

		m_hud->OnCharButtonClickedEvent.AddUniqueDynamic(m_voxtaClient, &UVoxtaClient::LoadCharacter);
		m_hud->OnUserInputCommittedEvent.AddUniqueDynamic(m_voxtaClient, &UVoxtaClient::SendUserInput);

		return true;
	}
	return false;
}

bool ATestGameCharacter::TryDisconnectToHud()
{
	if (m_voxtaClient && m_hud)
	{
		m_voxtaClient->OnVoxtaClientStateChangedDelegate.RemoveAll(this);
		m_voxtaClient->OnVoxtaClientCharacterLoadedDelegate.RemoveAll(this);
		m_voxtaClient->OnVoxtaClientChatMessageAdded.RemoveAll(this);
		m_voxtaClient->OnVoxtaClientChatMessageRemoved.RemoveAll(this);

		m_hud->OnCharButtonClickedEvent.RemoveAll(this);
		m_hud->OnUserInputCommittedEvent.RemoveAll(this);

		return true;
	}
	return false;
}