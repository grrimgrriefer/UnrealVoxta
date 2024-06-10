// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaClient.h"
#include "SignalRSubsystem.h"

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
	if (m_currentState != VoxtaClientState::Disconnected)
	{
		UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient has alread (tried to be) connected, ignoring new connection attempt");
		return;
	}

	m_currentState = VoxtaClientState::Connecting;
	m_hub = GEngine->GetEngineSubsystem<USignalRSubsystem>()->CreateHubConnection(FString::Format(*FString(TEXT("http://{0}:{1}/hub")), {
		TEXT("192.168.178.8"),
		TEXT("5384")
		}));

	m_hub->OnConnected().AddUObject(this, &UVoxtaClient::OnConnected);
	m_hub->OnConnectionError().AddUObject(this, &UVoxtaClient::OnConnectionError);
	m_hub->OnClosed().AddUObject(this, &UVoxtaClient::OnClosed);

	m_hub->Start();

	UE_LOGFMT(VoxtaLog, Log, "Starting Voxta client");
}

void UVoxtaClient::Disconnect()
{
	if (m_currentState == VoxtaClientState::Disconnected || m_currentState == VoxtaClientState::Terminated)
	{
		UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient is no longer connected, ignoring new disconnect attempt");
		return;
	}
	m_currentState = VoxtaClientState::Terminated;
	m_hub->Stop();
}

void UVoxtaClient::OnConnected()
{
	UE_LOGFMT(VoxtaLog, Log, "VoxtaClient connected successfully");
	SendMessageToServer(m_voxtaRequestApi.GetAuthenticateRequestData());
}

void UVoxtaClient::OnConnectionError(const FString& error)
{
	UE_LOGFMT(VoxtaLog, Error, "VoxtaClient connection has encountered error: {error}.", error);
	m_currentState = VoxtaClientState::Terminated;
}

void UVoxtaClient::OnClosed()
{
	UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient connection has been closed.");
	m_currentState = VoxtaClientState::Terminated;
}

void UVoxtaClient::SendMessageToServer(const FSignalRValue& message)
{
	m_hub->Invoke(m_sendMessageEventName, message).BindUObject(this, &UVoxtaClient::OnMessageSent);
}

void UVoxtaClient::OnMessageSent(const FSignalRInvokeResult& result)
{
	if (result.HasError())
	{
		UE_LOGFMT(VoxtaLog, Error, "Failed to send message due to error: {err}.", result.GetErrorMessage());
	}
}