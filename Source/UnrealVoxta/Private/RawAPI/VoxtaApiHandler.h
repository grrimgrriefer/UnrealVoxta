// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "VoxtaResponseDispatcher.h"
#include "VoxtaApiHandler.generated.h"

class UVoxtaSocketHandler;

DECLARE_MULTICAST_DELEGATE(FOnVoxtaConnected);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnVoxtaConnectionError, const FString&);
DECLARE_MULTICAST_DELEGATE(FOnVoxtaDisconnected);

/**
 * Lower level API, internal use only.
 * Owns the socket connection and handles the translation (serialization/deserialization) of requests and responses
 * between strongly-typed objects and raw string SignalR messages.
 * Helper class to serialize requests and deserialize responses using VoxtaPayloads.
 */
UCLASS()
class UVoxtaApiHandler : public UObject
{
	GENERATED_BODY()

public:
	static const FName CLIENT_NAME;
	static const FName CLIENT_VERSION;

	FOnVoxtaConnected m_OnConnected;
	FOnVoxtaConnectionError m_OnConnectionError;
	FOnVoxtaDisconnected m_OnDisconnected;

	void Initialize();
	void EstablishConnection(const FString& ipv4Address, int port) const;
	void Disconnect() const;

	bool TrySendAuthenticatePayload(const FName& clientName, const FName& version) const;
	bool TrySendLoadCharactersPayload() const;
	bool TrySendStartChatPayload(const FString& characterId) const;
	bool TrySendStopChatPayload(const FString& sessionId) const;
	bool TrySendSendTextMessagePayload(const FString& sessionId, const FString& text) const;

	template <typename T>
	TMulticastDelegate<void(const T&)>& OnResponse();

private:
	void OnSocketConnected();
	void OnSocketConnectionError(const FString& error);
	void OnSocketClosed();
	void OnMessageReceived(const FString& jsonString);

	template <class T>
	bool TrySendPayloadInternal(const T& payload, const TCHAR* payloadName) const;
	template <typename T>
	void RegisterResponseRoute(const FString& actionName);
	template <typename T>
	void BroadcastResponse(const T& response);

	template <class T>
	static bool ParseResponseInternal(const FString& jsonString, T& outResponse);

	static bool TryExtractAction(const FString& jsonString, FString& outAction);

	UPROPERTY()
	TObjectPtr<UVoxtaSocketHandler> m_voxtaSocketHandler;

	TMap<FString, TFunction<void(const FString&)>> m_responseRoutes;
	TMap<FName, TSharedPtr<IVoxtaResponseDispatcher>> m_responseDispatchers;

};
