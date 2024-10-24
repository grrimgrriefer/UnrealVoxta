// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "TestLogSink.h"
#include "CQTest.h"
#include "VoxtaClient.h"
#include "AiCharData.h"
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
	TArray<FChatMessage> m_messages;

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
		m_characterRegisteredEventHandle = m_voxtaClient->VoxtaClientCharacterRegisteredEventNative.AddLambda([this] (const FAiCharData& charData)
		{
			m_characters.Add(charData);
		});

		m_messages.Empty();
		m_charMessageAddedEventHandle = m_voxtaClient->VoxtaClientCharMessageAddedEventNative.AddLambda([this] (const FAiCharData& charData, const FChatMessage& message)
		{
			m_messages.Add(message);
		});
	}

	AFTER_EACH()
	{
		// Make sure that cleanup warnings don't affect the test outcome
		TestRunner->SetSuppressLogWarnings();

		m_voxtaClient->VoxtaClientStateChangedEventNative.Remove(m_stateChangedEventHandle);
		m_voxtaClient->VoxtaClientStateChangedEventNative.Remove(m_characterRegisteredEventHandle);
		m_voxtaClient->VoxtaClientStateChangedEventNative.Remove(m_charMessageAddedEventHandle);
		m_gameInstance->Shutdown();
		m_voxtaClient = nullptr;
		m_gameInstance = nullptr;

		m_newStateResponse = VoxtaClientState::Disconnected;

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
		/** Test */
		m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);

		/** Assert */
		GLog->Flush();
		ASSERT_THAT(AreEqual(m_newStateResponse, VoxtaClientState::AttemptingToConnect));
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::AttemptingToConnect));
	}

	TEST_METHOD(StartConnection_WithEmptyAddress_ExpectDisconnectedAndErrorLog)
	{
		/** Setup */
		TestRunner->SetSuppressLogErrors();

		/** Test */
		m_voxtaClient->StartConnection(FString(), 5384);

		/** Assert */
		GLog->Flush();
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Disconnected));
		ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("address"), ELogVerbosity::Type::Error)));
	}

	TEST_METHOD(StartConnection_WithInvalidAddress_ExpectDisconnectedAndErrorLog)
	{
		/** Setup */
		TestRunner->SetSuppressLogErrors();

		/** Test */
		m_voxtaClient->StartConnection(FString("1.2.3.4.5"), 5384);

		/** Assert */
		GLog->Flush();
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Disconnected));
		ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("address"), ELogVerbosity::Type::Error)));
	}

	TEST_METHOD(StartConnection_WithInvalidPort_ExpectDisconnectedAndErrorLog)
	{
		/** Setup */
		TestRunner->SetSuppressLogErrors();
		int port = 100000;

		/** Test */
		m_voxtaClient->StartConnection(FString("127.0.0.1"), port);

		/** Assert */
		GLog->Flush();
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Disconnected));
		ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(FString::FromInt(port), ELogVerbosity::Type::Error)));
	}

	TEST_METHOD(StartConnection_WithInvalidStartingState_ExpectIgnoredAndWarningLog)
	{
		/** Setup */
		PreconfigureClient(PreconfigureClientState::Started);
		TestRunner->SetSuppressLogWarnings();

		/** Test */
		m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);

		/** Assert */
		GLog->Flush();
		ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("ignoring new connection attempt"), ELogVerbosity::Type::Warning)));
	}

	TEST_METHOD(StartConnection_WaitForCharacterRegisteredEvent_ExpectEventTriggeredBeforeTimeout)
	{
		/** Setup */
		PreconfigureClient(PreconfigureClientState::Started);
		FTimespan timeout = FTimespan::FromSeconds(5);

		/** Test */
		AddCommand(new FWaitUntil(*TestRunner, [&] () { return m_voxtaClient->GetCurrentState() == VoxtaClientState::Idle; }, timeout));

		TestCommandBuilder.Do([this] ()
			{
				/** Assert */
				GLog->Flush();
				ASSERT_THAT(IsTrue(m_characters.Num() > 0));
				for (int i = 0; i < m_characters.Num(); i++)
				{
					ASSERT_THAT(IsNotNull(m_characters[i]));
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

		/** Test */
		m_voxtaClient->Disconnect();

		/** Assert */
		GLog->Flush();
		ASSERT_THAT(AreEqual(m_newStateResponse, VoxtaClientState::Terminated));
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Terminated));
	}

	TEST_METHOD(Disconnect_WithoutHavingConnected_ExpectedIgnoredAndWarningLog)
	{
		/** Setup */
		TestRunner->SetSuppressLogWarnings();

		/** Test */
		m_voxtaClient->Disconnect();

		/** Assert */
		GLog->Flush();
		ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("ignoring disconnect attempt"), ELogVerbosity::Type::Warning)));
	}

	TEST_METHOD(Disconnect_WithSilentEnabled_StateRemainsUnchanged)
	{
		/** Setup */
		PreconfigureClient(PreconfigureClientState::Started);
		VoxtaClientState state = m_voxtaClient->GetCurrentState();

		/** Test */
		m_voxtaClient->Disconnect(true);

		/** Assert */
		GLog->Flush();
		ASSERT_THAT(AreEqual(m_newStateResponse, state));
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), state));
	}
#pragma endregion

#pragma region StartChatWithCharacter
	TEST_METHOD(StartChatWithCharacter_WithInvalidStartingState_ExpectErrorAndStateRemainsUnchanged)
	{
		/** Setup */
		FString characterId = FGuid::NewGuid().ToString();
		VoxtaClientState state = m_voxtaClient->GetCurrentState();
		TestRunner->SetSuppressLogErrors();

		/** Test */
		m_voxtaClient->StartChatWithCharacter(characterId);

		/** Assert */
		GLog->Flush();
		ASSERT_THAT(AreEqual(m_newStateResponse, state));
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), state));
		ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(characterId, ELogVerbosity::Type::Error)));
	}

	TEST_METHOD(StartChatWithCharacter_WithEmptyString_ExpectErrorAndStateRemainsUnchanged)
	{
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			FString characterId = FString();
			VoxtaClientState state = m_voxtaClient->GetCurrentState();
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartChatWithCharacter(characterId);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_newStateResponse, state));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), state));
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

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_messages.Num(), 1));
			ASSERT_THAT(IsNotNull(m_messages[0]));
			ASSERT_THAT(IsNotNull(m_messages[0].GetCharId(), m_characters[0].GetId()));
		});
	}
#pragma endregion

#pragma region SendUserInput
	TEST_METHOD(SendUserInput_WithInvalidStartingState_ExpectErrorAndStateRemainsUnchanged)
	{
		/** Setup */
		FString text = TEXT("some user text");
		VoxtaClientState state = m_voxtaClient->GetCurrentState();
		TestRunner->SetSuppressLogErrors();

		/** Test */
		m_voxtaClient->SendUserInput(text);

		/** Assert */
		GLog->Flush();
		ASSERT_THAT(AreEqual(m_newStateResponse, state));
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), state));
		ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(text, ELogVerbosity::Type::Error)));
	}

	TEST_METHOD(SendUserInput_ExpectStateChange)
	{
		/** Setup */
		PreconfigureClient(PreconfigureClientState::ChatStarted);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->SendUserInput(TEXT("some user text"));

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

		TestCommandBuilder.Do([this, text] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_messages.Num(), 2));
			ASSERT_THAT(AreEqual(m_messages[1].GetTextContent(), text));
			ASSERT_THAT(AreEqual(m_messages[1].GetCharId(), m_voxtaClient->GetUserId()));
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
		m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
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
#pragma endregion
};