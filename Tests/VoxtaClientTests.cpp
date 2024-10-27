// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "TestLogSink.h"
#include "CQTest.h"
#include "VoxtaClient.h"
#include "VoxtaClientState.h"
#include "AiCharData.h"
#include "UserCharData.h"
#include "ChatMessage.h"
#include "Components/ActorTestSpawner.h"
#include "TestGameInstance.h"
#include "TestPlaybackActor.h"

#define PRE_TEST TestCommandBuilder.Do([this] () { TestRunner->SetSuppressLogWarnings(); TestRunner->SetSuppressLogErrors(); }); \
AddCommand(WaitForDuration(0.5)); \
TestCommandBuilder.Do([this] () { TestRunner->SetSuppressLogWarnings(ECQTestSuppressLogBehavior::False); TestRunner->SetSuppressLogErrors(ECQTestSuppressLogBehavior::False); });

/**
 * VoxtaClientTests
 * Tester class that will test the full public API of VoxtaClient.
 *
 * NOTE: These are integration tests and require VoxtaServer to be running on localhost.
 */
TEST_CLASS(VoxtaClientTests, "Voxta")
{
	UVoxtaClient* m_voxtaClient;
	TestLogSink* m_testLogSink;
	FActorTestSpawner* m_actorTestSpawner;

	FDelegateHandle m_stateChangedEventHandle;
	FDelegateHandle m_characterRegisteredEventHandle;
	FDelegateHandle m_charMessageAddedEventHandle;

	VoxtaClientState m_newStateResponse;
	TArray<FAiCharData> m_characters;
	TArray<TTuple<FBaseCharData, FChatMessage>> m_messages;

	VoxtaClientState m_cache_state;
	TWeakObjectPtr<ATestPlaybackActor> m_cache_playbackActor;

	/** Create a new clean m_gameInstance and voxtaclient for ever test, to avoid inter-test false positives. */
	BEFORE_EACH()
	{
		m_testLogSink = new TestLogSink();
		GLog->AddOutputDevice(m_testLogSink);
		TestRunner->SetSuppressLogErrors(ECQTestSuppressLogBehavior::False);
		TestRunner->SetSuppressLogWarnings(ECQTestSuppressLogBehavior::False);

		m_actorTestSpawner = new FActorTestSpawner();
		m_actorTestSpawner->InitializeGameSubsystems();
		m_voxtaClient = m_actorTestSpawner->GetGameInstance()->GetSubsystem<UVoxtaClient>();

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
		m_charMessageAddedEventHandle = m_voxtaClient->VoxtaClientCharMessageRemovedEventNative.AddLambda([this] (const FChatMessage& message)
		{
			int index = m_messages.IndexOfByPredicate([&message] (const TTuple<FBaseCharData, FChatMessage>& inItem)
			{
				return inItem.Value.GetMessageId() == message.GetMessageId();
			});
			m_messages.RemoveAt(index);
		});
	}

	AFTER_EACH()
	{
		// Make sure that cleanup warnings don't affect the test outcome
		TestRunner->SetSuppressLogWarnings();
		TestRunner->SetSuppressLogErrors();

		m_voxtaClient->VoxtaClientStateChangedEventNative.Remove(m_stateChangedEventHandle);
		m_voxtaClient->VoxtaClientCharacterRegisteredEventNative.Remove(m_characterRegisteredEventHandle);
		m_voxtaClient->VoxtaClientCharMessageAddedEventNative.Remove(m_charMessageAddedEventHandle);
		delete m_actorTestSpawner;
		m_voxtaClient = nullptr;
		m_actorTestSpawner = nullptr;

		m_newStateResponse = VoxtaClientState::Disconnected;
		m_characters.Empty();
		m_messages.Empty();

		GLog->Flush();
		GLog->RemoveOutputDevice(m_testLogSink);
		delete m_testLogSink;
	}

	TEST_METHOD(GetVoxtaSubsystem_ExpectNonNull)
	{
		PRE_TEST;
		ASSERT_THAT(IsNotNull(m_voxtaClient));
	}

	TEST_METHOD(GetVoxtaSubsystem_ExpectDisconnectedState)
	{
		PRE_TEST;
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Disconnected));
	}

#pragma region StartConnection
	TEST_METHOD(StartConnection_WithValidHost_NewStateAndBroadCast)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
		});

		AddCommand(WaitForDuration(0.5));
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
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartConnection(FString(), 5384);
		});

		AddCommand(WaitForDuration(0.5));
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
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartConnection(FString("1.2.3.4.5"), 5384);
		});

		AddCommand(WaitForDuration(0.5));
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
		PRE_TEST;
		/** Setup */
		int port = 100000;
		TestCommandBuilder.Do([this, port] ()
		{
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartConnection(FString("127.0.0.1"), port);
		});

		AddCommand(WaitForDuration(0.5));
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
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::Started);
		TestCommandBuilder.Do([this] ()
		{
			TestRunner->SetSuppressLogWarnings();

			/** Test */
			m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
		});

		AddCommand(WaitForDuration(0.5));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("ignoring new connection attempt"), ELogVerbosity::Type::Warning)));
		});
	}

	TEST_METHOD(StartConnection_WaitForCharacterRegisteredEvent_ExpectEventTriggeredBeforeTimeout)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::Started);
		FTimespan timeout = FTimespan::FromSeconds(5);

		/** Test */
		AddCommand(new FWaitUntil(*TestRunner, [&] () { return m_voxtaClient->GetCurrentState() == VoxtaClientState::Idle; }, timeout));

		AddCommand(WaitForDuration(0.5));
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
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::Started);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->Disconnect();
		});

		AddCommand(WaitForDuration(0.5));
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
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogWarnings();

			/** Test */
			m_voxtaClient->Disconnect();
		});

		AddCommand(WaitForDuration(0.5));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("ignoring disconnect attempt"), ELogVerbosity::Type::Warning)));
		});
	}

	TEST_METHOD(Disconnect_WithSilentEnabled_StateRemainsUnchanged)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::Started);
		TestCommandBuilder.Do([this] ()
		{
			m_cache_state = m_voxtaClient->GetCurrentState();

			/** Test */
			m_voxtaClient->Disconnect(true);
		});

		AddCommand(WaitForDuration(0.5));
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
		PRE_TEST;
		/** Setup */
		FString characterId = FGuid::NewGuid().ToString();
		TestCommandBuilder.Do([this, characterId] ()
		{
			m_cache_state = m_voxtaClient->GetCurrentState();
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartChatWithCharacter(characterId);
		});

		AddCommand(WaitForDuration(0.5));
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
		PRE_TEST;
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

		AddCommand(WaitForDuration(0.5));
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
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->StartChatWithCharacter(m_characters[0].GetId());
		});

		AddCommand(WaitForDuration(0.5));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_newStateResponse, VoxtaClientState::StartingChat));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::StartingChat));
		});
	}

	TEST_METHOD(StartChatWithCharacter_WaitForFirstMessage_ExpectStateChangeAndAudioGeneratedWarning)
	{
		PRE_TEST;
		/** Setup */
		FTimespan timeout = FTimespan::FromSeconds(10);
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			TestRunner->SetSuppressLogWarnings();
			m_voxtaClient->StartChatWithCharacter(m_characters[0].GetId());
		});

		/** Test */
		AddCommand(new FWaitUntil(*TestRunner, [&] () { return m_voxtaClient->GetCurrentState() == VoxtaClientState::WaitingForUserReponse; }, timeout));

		AddCommand(WaitForDuration(0.5));
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_messages.Num(), 1));
			ASSERT_THAT(AreEqual(m_messages[0].Value.GetCharId(), m_characters[0].GetId()));
			ASSERT_THAT(AreEqual(m_messages[0].Key.GetId(), m_characters[0].GetId()));
			ASSERT_THAT(AreEqual(m_messages[0].Key.GetName(), m_characters[0].GetName()));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring("Audio data was generated", ELogVerbosity::Type::Warning)));
		});
	}

	TEST_METHOD(StartChatWithCharacter_WaitForFirstMessageAudioPlaybackStart_ExpectStateChange)
	{
		PRE_TEST;
		/** Setup */
		FTimespan timeout = FTimespan::FromSeconds(10);
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_cache_playbackActor = &m_actorTestSpawner->SpawnActor<ATestPlaybackActor>();
			m_cache_playbackActor->Initialize(m_characters[0].GetId());
			m_voxtaClient->StartChatWithCharacter(m_characters[0].GetId());
		});

		/** Test */
		AddCommand(new FWaitUntil(*TestRunner, [&] () { return m_voxtaClient->GetCurrentState() == VoxtaClientState::AudioPlayback; }, timeout));

		AddCommand(WaitForDuration(0.5));
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
		PRE_TEST;
		FString text = TEXT("some user text");
		TestCommandBuilder.Do([this, text] ()
		{
			/** Setup */
			m_cache_state = m_voxtaClient->GetCurrentState();
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->SendUserInput(text);
		});

		AddCommand(WaitForDuration(0.5));
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
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::ChatStarted);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->SendUserInput(TEXT("some user text"));
		});

		AddCommand(WaitForDuration(0.5));
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
		PRE_TEST;
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

		AddCommand(WaitForDuration(0.5));
		TestCommandBuilder.Do([this, text] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_messages.Num(), 3)); // ai greeting, user, ai reply
			ASSERT_THAT(AreEqual(m_messages[1].Value.GetTextContent(), text));
			ASSERT_THAT(AreEqual(m_messages[1].Key.GetId(), m_voxtaClient->GetUserId()));
		});
	}
#pragma endregion

#pragma region utility
	enum class PreconfigureClientState : uint8
	{
		Started,
		CharacterListLoaded,
		AudioPlaybackStarted,
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
			m_cache_playbackActor = &m_actorTestSpawner->SpawnActor<ATestPlaybackActor>();
			m_cache_playbackActor->Initialize(m_characters[0].GetId());
			m_voxtaClient->StartChatWithCharacter(m_characters[0].GetId());
		});
		AddCommand(new FWaitUntil(*TestRunner, [&] () { return m_voxtaClient->GetCurrentState() == VoxtaClientState::AudioPlayback; }));
		if (state == PreconfigureClientState::AudioPlaybackStarted)
		{
			return;
		}
		FTimespan timeout = FTimespan::FromSeconds(5.0);
		AddCommand(new FWaitUntil(*TestRunner, [&] () { return m_voxtaClient->GetCurrentState() == VoxtaClientState::WaitingForUserReponse; }, timeout));
	}

	FWaitUntil* WaitForDuration(double seconds)
	{
		FDateTime endTime = FDateTime::Now() + FTimespan::FromSeconds(seconds);
		return new FWaitUntil(*TestRunner, [&] () { return FDateTime::Now() > endTime; }, FTimespan::FromSeconds(seconds + 5));
	}
#pragma endregion
};