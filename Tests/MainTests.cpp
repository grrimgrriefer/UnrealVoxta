// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "CoreMinimal.h"
#include "CQTest.h"
#include "VoxtaClient.h"

class FTestLogSink : public FOutputDevice
{
public:
	TMap<FString, ELogVerbosity::Type> LogMessages;

	virtual void Serialize(const TCHAR* message, ELogVerbosity::Type verbosity, const class FName& category) override
	{
		LogMessages.Add(FString(message), verbosity);
	}

	bool ContainsLogMessageWithSubstring(const FString& message, ELogVerbosity::Type type) const
	{
		for (const TPair<FString, ELogVerbosity::Type>& LogEntry : LogMessages)
		{
			if (LogEntry.Key.Contains(message))
			{
				return true;
			}
		}
		return false;
	}
};

/**
 * VoxtaClientTests
 * Tester class that will test the full public API of VoxtaClient.
 *
 * NOTE: These are integration tests and require VoxtaServer to be running on localhost.
 */
TEST_CLASS(VoxtaClientTests, "Game.Voxta")
{
	UGameInstance* m_gameInstance;
	UVoxtaClient* m_voxtaClient;
	FTestLogSink* m_testLogSink;

	FDelegateHandle m_handle;
	VoxtaClientState m_newStateResponse;

	/** Create a new clean m_gameInstance and voxtaclient for ever test, to avoid inter-test false positives. */
	BEFORE_EACH()
	{
		m_testLogSink = new FTestLogSink();
		GLog->AddOutputDevice(m_testLogSink);

		m_gameInstance = NewObject<UGameInstance>();
		m_gameInstance->Init();
		m_voxtaClient = m_gameInstance->GetSubsystem<UVoxtaClient>();

		m_newStateResponse = VoxtaClientState::Disconnected;
		m_handle = m_voxtaClient->VoxtaClientStateChangedEventNative.AddLambda([this] (VoxtaClientState newState)
		{
			m_newStateResponse = newState;
		});
	}

	AFTER_EACH()
	{
		m_newStateResponse = VoxtaClientState::Disconnected;
		m_voxtaClient->VoxtaClientStateChangedEventNative.Remove(m_handle);

		m_gameInstance->Shutdown();
		m_voxtaClient = nullptr;
		m_gameInstance = nullptr;
		GLog->RemoveOutputDevice(m_testLogSink);
		delete m_testLogSink;
	}

	TEST_METHOD(GetVoxtaSubsystem_ExpectNonNull)
	{
		ASSERT_THAT(IsNotNull(m_voxtaClient));
	}

	TEST_METHOD(GetVoxtaSubsystem_ExpectDisconnectedState)
	{
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Disconnected));
	}

#pragma region StartConnection
	TEST_METHOD(StartConnection_WithEmptyAddress_ExpectDisconnectedAndErrorLog)
	{
		m_voxtaClient->StartConnection(FString(), 5384);
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Disconnected));
		ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("address"), ELogVerbosity::Type::Error)));
	}

	TEST_METHOD(StartConnection_WithInvalidAddress_ExpectDisconnectedAndErrorLog)
	{
		m_voxtaClient->StartConnection(FString("500.500.500.500"), 5384);
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Disconnected));
		ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("address"), ELogVerbosity::Type::Error)));
	}

	TEST_METHOD(StartConnection_WithInvalidPort_ExpectDisconnectedAndErrorLog)
	{
		int port = 100000;
		m_voxtaClient->StartConnection(FString("127.0.0.1"), port);
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Disconnected));
		ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(FString::FromInt(port), ELogVerbosity::Type::Error)));
	}

	TEST_METHOD(StartConnection_WithInvalidStartingState_ExpectIgnoredAndWarningLog)
	{
		m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
		m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
		ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("ignoring new connection attempt"), ELogVerbosity::Type::Warning)));
	}

	TEST_METHOD(StartConnection_NewStateAndBroadCast)
	{
		m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
		ASSERT_THAT(AreEqual(m_newStateResponse, VoxtaClientState::AttemptingToConnect));
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::AttemptingToConnect));
	}
#pragma endregion

#pragma region Disconnect
	TEST_METHOD(Disconnect_WithoutHavingConnected_ExpectedIgnoredAndWarningLog)
	{
		m_voxtaClient->Disconnect();
		ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("ignoring disconnect attempt"), ELogVerbosity::Type::Warning)));
	}

	TEST_METHOD(Disconnect_ExpectNewStateAndBroadcast)
	{
		m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
		m_voxtaClient->Disconnect();
		ASSERT_THAT(AreEqual(m_newStateResponse, VoxtaClientState::Terminated));
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Terminated));
	}

	TEST_METHOD(Disconnect_WithSilentEnabled_StateRemainsUnchanged)
	{
		m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
		VoxtaClientState state = m_voxtaClient->GetCurrentState();

		m_voxtaClient->Disconnect();
		ASSERT_THAT(AreEqual(m_newStateResponse, state));
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), state));
	}
#pragma endregion
};