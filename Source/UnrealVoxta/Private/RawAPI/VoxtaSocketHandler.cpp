// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaSocketHandler.h"
#include "IHubConnection.h"
#include "SignalRSubsystem.h"
#include "Dom/JsonObject.h"
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
void UVoxtaSocketHandler::Disconnect() const
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
bool UVoxtaSocketHandler::TrySendPayload(const TSharedPtr<FJsonObject>& payload) const
{
	ensureAlways(m_hub.IsValid());
	if (m_hub.IsValid())
	{
		m_hub->Invoke(SEND_MESSAGE_EVENT_NAME, JsonObjectToSignalRValue(payload));
		return true;
	}
	return false;
}
void UVoxtaSocketHandler::OnConnected()
{
	m_OnSocketConnected.Broadcast();
}
void UVoxtaSocketHandler::OnConnectionError(const FString& error)
{
	m_OnSocketConnectionError.Broadcast(error);
}
void UVoxtaSocketHandler::OnClosed()
{
	m_OnSocketClosed.Broadcast();
}
void UVoxtaSocketHandler::OnReceivedMessage(const TArray<FSignalRValue>& payload)
{
	// TODO generic deserializeation
}
FSignalRValue UVoxtaSocketHandler::JsonValueToSignalRValue(const TSharedPtr<FJsonValue>& jsonValue) const
{
	if (!jsonValue.IsValid() || jsonValue->IsNull())
	{
		return FSignalRValue(nullptr);
	}

	switch (jsonValue->Type)
	{
	case EJson::Boolean:
		return FSignalRValue(jsonValue->AsBool());

	case EJson::Number:
		return FSignalRValue(jsonValue->AsNumber());

	case EJson::String:
		return FSignalRValue(jsonValue->AsString());

	case EJson::Array:
		{
			TArray<FSignalRValue> array;
			for (const TSharedPtr<FJsonValue>& item : jsonValue->AsArray())
			{
				array.Add(JsonValueToSignalRValue(item));
			}
			return FSignalRValue(MoveTemp(array));
		}

	case EJson::Object:
		{
			TMap<FString, FSignalRValue> map;
			for (const auto& pair : jsonValue->AsObject()->Values)
			{
				map.Add(FString(pair.Key), JsonValueToSignalRValue(pair.Value));
			}
			return FSignalRValue(MoveTemp(map));
		}

	default:
		return FSignalRValue(nullptr);
	}
}
FSignalRValue UVoxtaSocketHandler::JsonObjectToSignalRValue(const TSharedPtr<FJsonObject>& jsonObject) const
{
	if (!jsonObject.IsValid())
	{
		return FSignalRValue(nullptr);
	}

	TMap<FString, FSignalRValue> map;

	// Ensure that $type is the first one, because the server demands it to be first
	if (const TSharedPtr<FJsonValue> actionValue = jsonObject->TryGetField(TEXT("action")))
	{
		map.Add(TEXT("$type"), JsonValueToSignalRValue(actionValue));
	}
	else if (const TSharedPtr<FJsonValue> typeValue = jsonObject->TryGetField(TEXT("$type")))
	{
		map.Add(TEXT("$type"), JsonValueToSignalRValue(typeValue));
	}

	for (const auto& pair : jsonObject->Values)
	{
		if (pair.Key != TEXT("action") && pair.Key != TEXT("$type"))
		{
			map.Add(FString(pair.Key), JsonValueToSignalRValue(pair.Value));
		}
	}
	return FSignalRValue(MoveTemp(map));
}
