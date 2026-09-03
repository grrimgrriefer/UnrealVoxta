// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaSocketHandler.h"
#include "IHubConnection.h"
#include "SignalRSubsystem.h"
#include "Engine/Engine.h"

const FString UVoxtaSocketHandler::SEND_MESSAGE_EVENT_NAME = TEXT("SendMessage");
const FString UVoxtaSocketHandler::RECEIVE_MESSAGE_EVENT_NAME = TEXT("ReceiveMessage");

void UVoxtaSocketHandler::EstablishConnection(const FString& ipv4Address, int port)
{
	m_hub = GEngine->GetEngineSubsystem<USignalRSubsystem>()->CreateHubConnection(
		FString::Format(*FString(TEXT("http://{0}:{1}/hub")), {
			ipv4Address,
			port
		}));

	m_hub->OnConnected().AddUObject(this, &UVoxtaSocketHandler::OnConnected);
	m_hub->OnConnectionError().AddUObject(this, &UVoxtaSocketHandler::OnConnectionError);
	m_hub->OnClosed().AddUObject(this, &UVoxtaSocketHandler::OnClosed);
	m_hub->On(RECEIVE_MESSAGE_EVENT_NAME).BindUObject(this, &UVoxtaSocketHandler::OnReceivedMessage);
	m_hub->Start();
}
void UVoxtaSocketHandler::Disconnect()
{
	m_hub->OnConnected().RemoveAll(this);
	m_hub->OnConnectionError().RemoveAll(this);
	m_hub->OnClosed().RemoveAll(this);
	m_hub->On(RECEIVE_MESSAGE_EVENT_NAME).Unbind();

	if (m_hub.IsValid())
	{
		m_hub->Stop();
	}
}
bool UVoxtaSocketHandler::TrySendThroughSocket(const FString& message) const
{
	ensure(m_hub.IsValid());
	if (m_hub.IsValid())
	{
		m_hub->Invoke(SEND_MESSAGE_EVENT_NAME, message);
		return true;
	}
	return false;
}
void UVoxtaSocketHandler::OnConnected()
{
	// TODO notify the VoxtaConnectTask somehow
}
void UVoxtaSocketHandler::OnConnectionError(const FString& String)
{
	// TODO notify the VoxtaConnectTask somehow
}
void UVoxtaSocketHandler::OnClosed()
{
	// TODO notify the VoxtaConnectTask somehow
}
void UVoxtaSocketHandler::OnReceivedMessage(const TArray<FSignalRValue>& payload)
{
	// TODO generic deserializeation
}
