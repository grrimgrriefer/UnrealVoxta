// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaClient.h"
#include "SignalRSubsystem.h"
#include "VoxtaDefines.h"
#include "ServerResponseBase.h"
#include "ServerResponseWelcome.h"

UVoxtaClient::UVoxtaClient()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVoxtaClient::BeginPlay()
{
	Super::BeginPlay();
	m_logUtility.RegisterVoxtaLogger();
}

void UVoxtaClient::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (m_currentState != VoxtaClientState::Disconnected)
	{
		Disconnect();
	}
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
	m_hub = GEngine->GetEngineSubsystem<USignalRSubsystem>()->CreateHubConnection(FString::Format(*API_STRING("http://{0}:{1}/hub"), {
		TEXT("192.168.178.8"),
		TEXT("5384")
		}));

	StartListeningToServer();
	m_hub->Start();

	UE_LOGFMT(VoxtaLog, Log, "Starting Voxta client");
}

void UVoxtaClient::Disconnect()
{
	if (m_currentState == VoxtaClientState::Disconnected)
	{
		UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient is currently not connected, ignoring disconnect attempt");
		return;
	}
	m_currentState = VoxtaClientState::Terminated;
	m_hub->Stop();
}

void UVoxtaClient::StartListeningToServer()
{
	m_hub->On(m_receiveMessageEventName).BindUObject(this, &UVoxtaClient::OnReceivedMessage);
	m_hub->OnConnected().AddUObject(this, &UVoxtaClient::OnConnected);
	m_hub->OnConnectionError().AddUObject(this, &UVoxtaClient::OnConnectionError);
	m_hub->OnClosed().AddUObject(this, &UVoxtaClient::OnClosed);
}

void UVoxtaClient::OnReceivedMessage(const TArray<FSignalRValue>& Arguments)
{
	if (m_currentState == VoxtaClientState::Disconnected || m_currentState == VoxtaClientState::Terminated)
	{
		UE_LOGFMT(VoxtaLog, Warning, "Tried to process a message with the connection already severed, skipping processing of remaining response data.");
		return;
	}
	if (Arguments.IsEmpty() || Arguments[0].GetType() != FSignalRValue::EValueType::Object)
	{
		UE_LOGFMT(VoxtaLog, Error, "Received invalid message from server.");
	}
	else try
	{
		if (!HandleResponse(Arguments[0].AsObject()))
		{
			UE_LOGFMT(VoxtaLog, Warning, "Received server response that is not (yet) supported.");
		}
	}
	catch (const std::exception& ex)
	{
		FString error(TEXT("Something went wrong while parsing the server response: "));
		error += ex.what();
		UE_LOGFMT(VoxtaLog, Error, "Received server response that is not (yet) supported.", error);
	}
}

void UVoxtaClient::OnConnected()
{
	UE_LOGFMT(VoxtaLog, Log, "VoxtaClient connected successfully");
	SendMessageToServer(m_voxtaRequestApi.GetAuthenticateRequestData());
}

void UVoxtaClient::OnConnectionError(const FString& error)
{
	UE_LOGFMT(VoxtaLog, Error, "VoxtaClient connection has encountered error: {error}.", error);
	Disconnect();
}

void UVoxtaClient::OnClosed()
{
	UE_LOGFMT(VoxtaLog, Warning, "VoxtaClient connection has been closed.");
	Disconnect();
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

bool UVoxtaClient::HandleResponse(const TMap<FString, FSignalRValue>& responseData)
{
	if (m_voxtaResponseApi.ignoredMessageTypes.Contains(responseData[API_STRING("$type")].AsString()))
	{
		return true;
	}

	auto response = m_voxtaResponseApi.GetResponseData(responseData);
	if (!response)
	{
		return false;
	}

	switch (response->GetType())
	{
		using enum ServerResponseType;
		case Welcome:
		{
			UE_LOGFMT(VoxtaLog, Log, "Logged in sucessfully");
			HandleWelcomeResponse(*response);
			return true;
		}
		case CharacterList:
		case CharacterLoaded:
		case ChatStarted:
		case ChatMessage:
		case ChatUpdate:
		case SpeechTranscription:
		default:
			return false;
	}
}

void UVoxtaClient::HandleWelcomeResponse(const ServerResponseBase& response)
{
	auto derivedResponse = dynamic_cast<const ServerResponseWelcome*>(&response);
	m_userData = MakeUnique<CharData>(derivedResponse->m_user);
	UE_LOGFMT(VoxtaLog, Log, "Authenticated with Voxta Server. Welcome {user}.", m_userData->m_name);
	m_currentState = VoxtaClientState::Authenticated;
	SendMessageToServer(m_voxtaRequestApi.GetLoadCharactersListData());
}