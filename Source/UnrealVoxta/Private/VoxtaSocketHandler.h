// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "VoxtaSocketHandler.generated.h"

class FSignalRValue;
class AGameModeBase;
class IHubConnection;
class APlayerController;
struct FGameplayTag;

/**
 * Owns the low-level socket.
 * Is controlled by the Tasks in the StateTree of the VoxtaSubsystem.
 */
UCLASS()
class UVoxtaSocketHandler : public UObject
{
	GENERATED_BODY()

public:
	void EstablishConnection(const FString& ipv4Address, int port);
	void Disconnect();
	bool TrySendThroughSocket(const FString& message) const;

private:
	static const FString SEND_MESSAGE_EVENT_NAME;
	static const FString RECEIVE_MESSAGE_EVENT_NAME;

	void OnConnected();
	void OnConnectionError(const FString& String);
	void OnClosed();
	void OnReceivedMessage(const TArray<FSignalRValue>& payload);

	TSharedPtr<IHubConnection> m_hub;
};
