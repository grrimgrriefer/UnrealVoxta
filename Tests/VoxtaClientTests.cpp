// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "TestLogSink.h"
#include "CQTest.h"
#include "VoxtaClient.h"
#include "VoxtaClientState.h"
#include "AiCharData.h"
#include "UserCharData.h"
#include "ChatMessage.h"

/**
 * VoxtaClientTests
 * Tester class that will test the full public API of VoxtaClient.
 *
 * NOTE: These are integration tests and require VoxtaServer to be running on localhost.
 */
TEST_CLASS(VoxtaClientTests, "Voxta")
{
	UGameInstance* m_gameInstance;
	UVoxtaClient* m_voxtaClient;
	TestLogSink* m_testLogSink;

	FDelegateHandle m_stateChangedEventHandle;
	FDelegateHandle m_characterRegisteredEventHandle;
	FDelegateHandle m_charMessageAddedEventHandle;

	VoxtaClientState m_newStateResponse;
	TArray<FAiCharData> m_characters;
	TArray<TTuple<FBaseCharData, FChatMessage>> m_messages;

	VoxtaClientState m_cache_state;

	/** Create a new clean m_gameInstance and voxtaclient for ever test, to avoid inter-test false positives. */
	BEFORE_EACH()
	{
		m_testLogSink = new TestLogSink();
		GLog->AddOutputDevice(m_testLogSink);

		m_gameInstance = NewObject<UGameInstance>();
		m_gameInstance->Init();
		m_voxtaClient = m_gameInstance->GetSubsystem<UVoxtaClient>();

		m_newStateResponse = VoxtaClientState::Disconnected;
		m_stateChangedEventHandle = m_voxtaClient->VoxtaClientStateChangedEventNative.AddLambda([this] (VoxtaClientState newState)
		{
			m_newStateResponse = newState;
		});

		m_characters.Empty();
		m_characterRegisteredEventHandle = m_voxtaClient->VoxtaClientCharacterRegisteredEventNative.AddLambda([this] (const FAiCharData& aiCharData)
		{
			m_characters.Add(aiCharData);
		});

		m_messages.Empty();
		m_charMessageAddedEventHandle = m_voxtaClient->VoxtaClientCharMessageAddedEventNative.AddLambda([this] (const FBaseCharData& baseCharData, const FChatMessage& message)
		{
			m_messages.Add(MakeTuple(baseCharData, message));
		});
	}

	AFTER_EACH()
	{
		// Make sure that cleanup warnings don't affect the test outcome
		TestRunner->SetSuppressLogWarnings();

		m_voxtaClient->VoxtaClientStateChangedEventNative.Remove(m_stateChangedEventHandle);
		m_voxtaClient->VoxtaClientCharacterRegisteredEventNative.Remove(m_characterRegisteredEventHandle);
		m_voxtaClient->VoxtaClientCharMessageAddedEventNative.Remove(m_charMessageAddedEventHandle);
		m_gameInstance->Shutdown();
		m_voxtaClient = nullptr;
		m_gameInstance = nullptr;

		m_newStateResponse = VoxtaClientState::Disconnected;
		m_characters.Empty();
		m_messages.Empty();

		GLog->Flush();
		GLog->RemoveOutputDevice(m_testLogSink);
		TestRunner->SetSuppressLogErrors(ECQTestSuppressLogBehavior::False);
		TestRunner->SetSuppressLogWarnings(ECQTestSuppressLogBehavior::False);
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
	TEST_METHOD(StartConnection_WithValidHost_NewStateAndBroadCast)
	{
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_newStateResponse, VoxtaClientState::AttemptingToConnect));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::AttemptingToConnect));
		});
	}

	TEST_METHOD(StartConnection_WithEmptyAddress_ExpectDisconnectedAndErrorLog)
	{
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartConnection(FString(), 5384);
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Disconnected));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("address"), ELogVerbosity::Type::Error)));
		});
	}

	TEST_METHOD(StartConnection_WithInvalidAddress_ExpectDisconnectedAndErrorLog)
	{
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartConnection(FString("1.2.3.4.5"), 5384);
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Disconnected));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("address"), ELogVerbosity::Type::Error)));
		});
	}

	TEST_METHOD(StartConnection_WithInvalidPort_ExpectDisconnectedAndErrorLog)
	{
		/** Setup */
		int port = 100000;
		TestCommandBuilder.Do([this, port] ()
		{
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartConnection(FString("127.0.0.1"), port);
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this, port] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Disconnected));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(FString::FromInt(port), ELogVerbosity::Type::Error)));
		});
	}

	TEST_METHOD(StartConnection_WithInvalidStartingState_ExpectIgnoredAndWarningLog)
	{
		/** Setup */
		PreconfigureClient(PreconfigureClientState::Started);
		TestCommandBuilder.Do([this] ()
		{
			TestRunner->SetSuppressLogWarnings();

			/** Test */
			m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("ignoring new connection attempt"), ELogVerbosity::Type::Warning)));
		});
	}

	TEST_METHOD(StartConnection_WaitForCharacterRegisteredEvent_ExpectEventTriggeredBeforeTimeout)
	{
		/** Setup */
		PreconfigureClient(PreconfigureClientState::Started);
		FTimespan timeout = FTimespan::FromSeconds(5);

		/** Test */
		AddCommand(new FWaitUntil(*TestRunner, [&] () { return m_voxtaClient->GetCurrentState() == VoxtaClientState::Idle; }, timeout));

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(m_characters.Num() > 0));
			for (int i = 0; i < m_characters.Num(); i++)
			{
				ASSERT_THAT(IsNotNull(m_characters[i].GetName()));
			}
		});
	}
#pragma endregion

#pragma region Disconnect
	TEST_METHOD(Disconnect_WithValidState_ExpectNewStateAndBroadcast)
	{
		/** Setup */
		PreconfigureClient(PreconfigureClientState::Started);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->Disconnect();
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_newStateResponse, VoxtaClientState::Terminated));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Terminated));
		});
	}

	TEST_METHOD(Disconnect_WithoutHavingConnected_ExpectedIgnoredAndWarningLog)
	{
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogWarnings();

			/** Test */
			m_voxtaClient->Disconnect();
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("ignoring disconnect attempt"), ELogVerbosity::Type::Warning)));
		});
	}

	TEST_METHOD(Disconnect_WithSilentEnabled_StateRemainsUnchanged)
	{
		/** Setup */
		PreconfigureClient(PreconfigureClientState::Started);
		TestCommandBuilder.Do([this] ()
		{
			m_cache_state = m_voxtaClient->GetCurrentState();

			/** Test */
			m_voxtaClient->Disconnect(true);
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_newStateResponse, m_cache_state));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), m_cache_state));
		});
	}
#pragma endregion

#pragma region StartChatWithCharacter
	TEST_METHOD(StartChatWithCharacter_WithInvalidStartingState_ExpectErrorAndStateRemainsUnchanged)
	{
		/** Setup */
		FString characterId = FGuid::NewGuid().ToString();
		TestCommandBuilder.Do([this, characterId] ()
		{
			m_cache_state = m_voxtaClient->GetCurrentState();
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartChatWithCharacter(characterId);
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this, characterId] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_newStateResponse, m_cache_state));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), m_cache_state));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(characterId, ELogVerbosity::Type::Error)));
		});
	}

	TEST_METHOD(StartChatWithCharacter_WithEmptyString_ExpectErrorAndStateRemainsUnchanged)
	{
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			FString characterId = FString();
			m_cache_state = m_voxtaClient->GetCurrentState();
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartChatWithCharacter(characterId);
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_newStateResponse, m_cache_state));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), m_cache_state));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("empty"), ELogVerbosity::Type::Error)));
		});
	}

	TEST_METHOD(StartChatWithCharacter_WithValidCharID_ExpectStateChange)
	{
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->StartChatWithCharacter(m_characters[0].GetId());
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_newStateResponse, VoxtaClientState::StartingChat));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::StartingChat));
		});
	}

	TEST_METHOD(StartChatWithCharacter_WaitForFirstMessage_ExpectStateChange)
	{
		/** Setup */
		FTimespan timeout = FTimespan::FromSeconds(10);
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->StartChatWithCharacter(m_characters[0].GetId());
		});

		/** Test */
		AddCommand(new FWaitUntil(*TestRunner, [&] () { return m_voxtaClient->GetCurrentState() == VoxtaClientState::WaitingForUserReponse; }, timeout));

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_messages.Num(), 1));
			ASSERT_THAT(AreEqual(m_messages[0].Value.GetCharId(), m_characters[0].GetId()));
			ASSERT_THAT(AreEqual(m_messages[0].Key.GetId(), m_characters[0].GetId()));
			ASSERT_THAT(AreEqual(m_messages[0].Key.GetName(), m_characters[0].GetName()));
		});
	}
#pragma endregion

#pragma region SendUserInput
	TEST_METHOD(SendUserInput_WithInvalidStartingState_ExpectErrorAndStateRemainsUnchanged)
	{
		FString text = TEXT("some user text");
		TestCommandBuilder.Do([this, text] ()
		{
			/** Setup */
			m_cache_state = m_voxtaClient->GetCurrentState();
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->SendUserInput(text);
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this, text] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_newStateResponse, m_cache_state));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), m_cache_state));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(text, ELogVerbosity::Type::Error)));
		});
	}

	TEST_METHOD(SendUserInput_ExpectStateChange)
	{
		/** Setup */
		PreconfigureClient(PreconfigureClientState::ChatStarted);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->SendUserInput(TEXT("some user text"));
		});

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_newStateResponse, VoxtaClientState::GeneratingReply));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::GeneratingReply));
		});
	}

	TEST_METHOD(SendUserInput_WaitForReply_ExpectMessageBroadcast)
	{
		/** Setup */
		FTimespan timeout = FTimespan::FromSeconds(10);
		FString text = TEXT("Hey there!");
		PreconfigureClient(PreconfigureClientState::ChatStarted);
		TestCommandBuilder.Do([this, text] ()
		{
			m_voxtaClient->SendUserInput(text);
		});

		/** Test */
		AddCommand(new FWaitUntil(*TestRunner, [&] () { return m_voxtaClient->GetCurrentState() == VoxtaClientState::WaitingForUserReponse; }, timeout));

		AddCommand(WaitForDuration(0.1));
		TestCommandBuilder.Do([this, text] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_messages.Num(), 2));
			ASSERT_THAT(AreEqual(m_messages[1].Value.GetTextContent(), text));
			ASSERT_THAT(AreEqual(m_messages[1].Key.GetId(), m_voxtaClient->GetUserId()));
			ASSERT_THAT(IsTrue(m_messages[1].Key.StaticStruct() == FUserCharData::StaticStruct()));
		});
	}
#pragma endregion

#pragma region utility
	enum class PreconfigureClientState : uint8
	{
		Started,
		CharacterListLoaded,
		ChatStarted
	};

	void PreconfigureClient(PreconfigureClientState state)
	{
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
		});
		if (state == PreconfigureClientState::Started)
		{
			return;
		}
		AddCommand(new FWaitUntil(*TestRunner, [&] () { return m_voxtaClient->GetCurrentState() == VoxtaClientState::Idle; }));
		if (state == PreconfigureClientState::CharacterListLoaded)
		{
			return;
		}
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->StartChatWithCharacter(m_characters[0].GetId());
		});
		AddCommand(new FWaitUntil(*TestRunner, [&] () { return m_voxtaClient->GetCurrentState() == VoxtaClientState::WaitingForUserReponse; }));
	}

	FWaitUntil* WaitForDuration(double seconds)
	{
		FDateTime endTime = FDateTime::Now() + FTimespan::FromSeconds(seconds);
		return new FWaitUntil(*TestRunner, [&] ()
			{
				return FDateTime::Now() > endTime;
			}, FTimespan::FromSeconds(seconds + 5));
	}
#pragma endregion
};