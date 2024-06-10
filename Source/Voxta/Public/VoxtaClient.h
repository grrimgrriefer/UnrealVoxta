// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Voxta/Private/VoxtaLogUtility.h"
#include "SignalR/Private/HubConnection.h"
#include "Voxta/Private/VoxtaApiRequestHandler.h"
#include "VoxtaClient.generated.h"

UENUM(BlueprintType)
enum class VoxtaClientState : uint8
{
	Disconnected		UMETA(DisplayName = "Disconnected"),
	Connecting			UMETA(DisplayName = "Connecting"),
	Authenticated		UMETA(DisplayName = "Authenticated"),
	CharacterLobby		UMETA(DisplayName = "CharacterLobby"),
	Chatting			UMETA(DisplayName = "Chatting"),
	Terminated			UMETA(DisplayName = "Terminated")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VOXTA_API UVoxtaClient : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxtaClient();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

public:
	void StartConnection();
	void Disconnect();

private:
	VoxtaLogUtility m_logUtility;
	TSharedPtr<IHubConnection> m_hub;
	VoxtaApiRequestHandler m_voxtaRequestApi;

	VoxtaClientState m_currentState = VoxtaClientState::Disconnected;
	const FString m_sendMessageEventName = TEXT("SendMessage");

	void OnConnected();
	void OnConnectionError(const FString& error);
	void OnClosed();

	void SendMessageToServer(const FSignalRValue& message);
	void OnMessageSent(const FSignalRInvokeResult& result);
};
