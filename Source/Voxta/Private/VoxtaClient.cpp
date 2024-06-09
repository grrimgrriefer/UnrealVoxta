// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaClient.h"
#include <SignalRSubsystem.h>

UVoxtaClient::UVoxtaClient()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVoxtaClient::BeginPlay()
{
	Super::BeginPlay();
	m_logUtility.RegisterVoxtaLogger();
}

void UVoxtaClient::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UVoxtaClient::StartConnection()
{
	UE_LOGFMT(VoxtaLog, Log, "Starting Voxta client");
	m_hub = GEngine->GetEngineSubsystem<USignalRSubsystem>()->CreateHubConnection(TEXT("http://{}:{}/hub"));
}