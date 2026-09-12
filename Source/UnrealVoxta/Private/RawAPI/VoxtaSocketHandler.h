// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "VoxtaSocketHandler.generated.h"

class FJsonObject;
class FSignalRValue;
class AGameModeBase;
class IHubConnection;
class APlayerController;
struct FGameplayTag;

DECLARE_MULTICAST_DELEGATE(FOnVoxtaSocketConnected);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnVoxtaSocketConnectionError, const FString&);
DECLARE_MULTICAST_DELEGATE(FOnVoxtaSocketClosed);

/**
 * Owns the low-level socket.
 * Is controlled by the Tasks in the StateTree of the VoxtaSubsystem.
 */
UCLASS()
class UVoxtaSocketHandler : public UObject
{
	GENERATED_BODY()

public:
	FOnVoxtaSocketConnected m_OnSocketConnected;
	FOnVoxtaSocketConnectionError m_OnSocketConnectionError;
	FOnVoxtaSocketClosed m_OnSocketClosed;

	void EstablishConnection(const FString& ipv4Address, int port);
	void Disconnect() const;
	bool TrySendPayload(const FJsonObject* payload) const;

private:
	static const FString SEND_MESSAGE_EVENT_NAME;
	static const FString RECEIVE_MESSAGE_EVENT_NAME;

	void OnConnected();
	void OnConnectionError(const FString& error);
	void OnClosed();
	void OnReceivedMessage(const TArray<FSignalRValue>& payload);

	FSignalRValue JsonValueToSignalRValue(const FJsonValue* jsonValue) const;
	FSignalRValue JsonObjectToSignalRValue(const FJsonObject* jsonObject) const;

	TSharedPtr<IHubConnection> m_hub;
};
