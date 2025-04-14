// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "TestLogSink.h"
#include "CQTest.h"
#include "VoxtaClient.h"
#include "VoxtaClientState.h"
#include "VoxtaAudioPlayback.h"
#include "AiCharData.h"
#include "UserCharData.h"
#include "ChatMessage.h"
#include "Components/ActorTestSpawner.h"
#include "TestGameInstance.h"
#include "TestPlaybackActor.h"

#define EMPTY_TEXT TEXT("")
#define TEST_DELAY_SECONDS 1
#define UNRESPONSIVE_TIMEOUT (10 + TEST_DELAY_SECONDS)
#define TEST_DELAY TestCommandBuilder.Do([this] () { m_endTime = FDateTime::MaxValue(); }); \
WaitForFixedDuration(TEST_DELAY_SECONDS);
#define PRE_TEST TestCommandBuilder.Do([this] () { TestRunner->SetSuppressLogWarnings(ECQTestSuppressLogBehavior::True); TestRunner->SetSuppressLogErrors(ECQTestSuppressLogBehavior::True); }); \
TEST_DELAY \
TestCommandBuilder.Do([this] () { GLog->Flush(); }); \
TEST_DELAY \
TestCommandBuilder.Do([this] () { TestRunner->SetSuppressLogWarnings(ECQTestSuppressLogBehavior::False); TestRunner->SetSuppressLogErrors(ECQTestSuppressLogBehavior::False); });

/**
 * VoxtaClientTests
 * Tester class that will test the public API of VoxtaClient.
 *
 * NOTE: These are integration tests and require VoxtaServer to be running on localhost (127.0.0.1)
 */
TEST_CLASS(VoxtaClientTests, "Voxta")
{
	const FGuid m_voxtaId = FGuid(TEXT("35c74d75-e3e4-44af-9389-faade99cc419")); // Id of Voxta character
	const FString m_voxtaName = TEXT("Voxta");
	const FString m_signalRLogCategory = TEXT("LogSignalR");

	UVoxtaClient* m_voxtaClient;
	TestLogSink* m_testLogSink;
	FActorTestSpawner* m_actorTestSpawner;

	FDelegateHandle m_stateChangedEventHandle;
	FDelegateHandle m_characterRegisteredEventHandle;
	FDelegateHandle m_charMessageAddedEventHandle;
	FDelegateHandle m_audioPlaybackRegisteredEventHandle;

	VoxtaClientState m_latestNewStateResponse;
	TArray<VoxtaClientState> m_orderedListOfNewStateResponses;
	TArray<FAiCharData> m_characters;
	TArray<TTuple<FBaseCharData, FChatMessage>> m_messages;
	TArray<TTuple<const UVoxtaAudioPlayback*, FGuid>> m_playbackHandlers;

	VoxtaClientState m_cache_state;
	ATestPlaybackActor* m_cache_playbackActor;
	FGuid m_cache_guid;

	FDateTime m_endTime = FDateTime::MaxValue();

	/** Create a new clean m_gameInstance and voxtaclient for ever test, to avoid inter-test false positives. */
	BEFORE_EACH()
	{
		TestRunner->SuppressedLogCategories.Add(m_signalRLogCategory);

		m_testLogSink = new TestLogSink();
		GLog->AddOutputDevice(m_testLogSink);
		TestRunner->SetSuppressLogErrors(ECQTestSuppressLogBehavior::False);
		TestRunner->SetSuppressLogWarnings(ECQTestSuppressLogBehavior::False);

		m_actorTestSpawner = new FActorTestSpawner();
		m_actorTestSpawner->InitializeGameSubsystems();
		m_voxtaClient = m_actorTestSpawner->GetGameInstance()->GetSubsystem<UVoxtaClient>();
		m_endTime = FDateTime::MaxValue();

		m_latestNewStateResponse = VoxtaClientState::Disconnected;
		m_stateChangedEventHandle = m_voxtaClient->VoxtaClientStateChangedEventNative.AddLambda([this] (VoxtaClientState newState)
		{
			m_latestNewStateResponse = newState;
			m_orderedListOfNewStateResponses.Emplace(newState);
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

		m_playbackHandlers.Empty();
		m_audioPlaybackRegisteredEventHandle = m_voxtaClient->VoxtaClientAudioPlaybackRegisteredEventNative.AddLambda([this] (const UVoxtaAudioPlayback* playbackHandler, const FGuid& characterId)
		{
			m_playbackHandlers.Add(MakeTuple(playbackHandler, characterId));
		});

		m_cache_state = VoxtaClientState::Disconnected;
		m_cache_playbackActor = nullptr;
		m_cache_guid.Invalidate();
	}

	AFTER_EACH()
	{
		// Make sure that cleanup warnings don't affect the test outcome
		TestRunner->SetSuppressLogWarnings(ECQTestSuppressLogBehavior::True);
		TestRunner->SetSuppressLogErrors(ECQTestSuppressLogBehavior::True);

		m_voxtaClient->VoxtaClientStateChangedEventNative.Remove(m_stateChangedEventHandle);
		m_voxtaClient->VoxtaClientCharacterRegisteredEventNative.Remove(m_characterRegisteredEventHandle);
		m_voxtaClient->VoxtaClientCharMessageAddedEventNative.Remove(m_charMessageAddedEventHandle);
		m_voxtaClient->VoxtaClientAudioPlaybackRegisteredEventNative.Remove(m_charMessageAddedEventHandle);
		m_voxtaClient->Disconnect();
		m_voxtaClient = nullptr;
		m_actorTestSpawner = nullptr;
		m_cache_playbackActor = nullptr;
		m_cache_guid.Invalidate();

		m_latestNewStateResponse = VoxtaClientState::Disconnected;
		m_orderedListOfNewStateResponses.Empty();
		m_characters.Empty();
		m_messages.Empty();
		m_playbackHandlers.Empty();

		m_cache_state = VoxtaClientState::Disconnected;

		GLog->Flush();
		GLog->RemoveOutputDevice(m_testLogSink);
		delete m_testLogSink;

		TestRunner->SuppressedLogCategories.Remove(m_signalRLogCategory);
	}

#pragma region Validations
	TEST_METHOD(Validate_GetVoxtaSubsystem_ExpectNonNull)
	{
		PRE_TEST;
		ASSERT_THAT(IsNotNull(m_voxtaClient));
	}

	TEST_METHOD(Validate_GetVoxtaSubsystem_ExpectDisconnectedState)
	{
		PRE_TEST;
		ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Disconnected));
	}

	TEST_METHOD(Validate_ExpectVoxtaDefaultAssistantToExist)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();

			int indexVoxtaName = -1;
			int indexVoxtaId = -1;
			for (int i = 0; i < m_characters.Num(); i++)
			{
				if (m_characters[i].GetName() == m_voxtaName)
				{
					indexVoxtaName = i;
				}
				if (m_characters[i].GetId() == m_voxtaId)
				{
					indexVoxtaId = i;
				}
			}

			ASSERT_THAT(IsTrue(indexVoxtaName >= 0));
			ASSERT_THAT(IsTrue(indexVoxtaId >= 0));
			ASSERT_THAT(AreEqual(indexVoxtaName, indexVoxtaId));
			ASSERT_THAT(IsTrue(m_characters[indexVoxtaId].GetId().IsValid()));
		});
	}

	TEST_METHOD(Validate_ExpectAllCharactersHaveUniqueIDs)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();

			TSet<FGuid> idSet;
			bool alreadyExists = false;
			for (int i = 0; i < m_characters.Num(); i++)
			{
				idSet.Add(m_characters[i].GetId(), &alreadyExists);
				ASSERT_THAT(IsFalse(alreadyExists));
				ASSERT_THAT(IsTrue(m_characters[i].GetId().IsValid()));
			}
		});
	}
#pragma endregion

#pragma region StartConnection
	TEST_METHOD(StartConnection_WithValidHost_NewStateAndBroadCast)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
		});

		WaitUntilState(VoxtaClientState::Idle);
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::Idle));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Idle));
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

		WaitUntilState(VoxtaClientState::Disconnected);
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::Disconnected));
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

		WaitUntilState(VoxtaClientState::Disconnected);
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::Disconnected));
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

		WaitUntilState(VoxtaClientState::Disconnected);
		TestCommandBuilder.Do([this, port] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::Disconnected));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(FString::FromInt(port), ELogVerbosity::Type::Warning)));
		});
	}

	TEST_METHOD(StartConnection_WithInvalidStartingState_ExpectIgnoredAndWarningLog)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::Started);
		TestCommandBuilder.Do([this] ()
		{
			TestRunner->SetSuppressLogWarnings(ECQTestSuppressLogBehavior::True);

			/** Test */
			m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);
		
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

		/** Test */
		WaitUntilState(VoxtaClientState::Idle);

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::Idle));
			ASSERT_THAT(IsTrue(m_characters.Num() > 0));
			for (int i = 0; i < m_characters.Num(); i++)
			{
				ASSERT_THAT(IsNotNull(m_characters[i].GetName()));
				ASSERT_THAT(IsTrue(m_characters[i].GetId().IsValid()));
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

		WaitUntilState(VoxtaClientState::Terminated);
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::Terminated));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::Terminated));
		});
	}

	TEST_METHOD(Disconnect_WithoutHavingConnected_ExpectedIgnoredAndWarningLog)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogWarnings(ECQTestSuppressLogBehavior::True);

			/** Test */
			m_voxtaClient->Disconnect();
	
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

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, m_cache_state));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), m_cache_state));
		});
	}
#pragma endregion

#pragma region StartChatWithCharacter
	TEST_METHOD(StartChatWithCharacter_WithInvalidStartingState_ExpectErrorAndStateRemainsUnchanged)
	{
		PRE_TEST;
		/** Setup */
		FGuid characterId = FGuid::NewGuid();
		TestCommandBuilder.Do([this, characterId] ()
		{
			m_cache_state = m_voxtaClient->GetCurrentState();
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartChatWithCharacter(characterId);
		});

		TEST_DELAY;
		TestCommandBuilder.Do([this, characterId] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, m_cache_state));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), m_cache_state));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(characterId, ELogVerbosity::Type::Error)));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring("Disconnected", ELogVerbosity::Type::Error)));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

	TEST_METHOD(StartChatWithCharacter_WithEmptyString_ExpectErrorAndStateRemainsUnchanged)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_cache_state = m_voxtaClient->GetCurrentState();
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartChatWithCharacter(FGuid());

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, m_cache_state));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), m_cache_state));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("empty"), ELogVerbosity::Type::Error)));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

	TEST_METHOD(StartChatWithCharacter_WithInvalidCharacterId_ExpectErrorAndStateRemainsUnchanged)
	{
		PRE_TEST;
		/** Setup */
		FGuid characterId = FGuid::NewGuid();
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this, characterId] ()
		{
			m_cache_state = m_voxtaClient->GetCurrentState();
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->StartChatWithCharacter(characterId);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, m_cache_state));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), m_cache_state));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(characterId, ELogVerbosity::Type::Error)));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

	TEST_METHOD(StartChatWithCharacter_WithValidCharID_ExpectStateChange)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});

		WaitUntilState(VoxtaClientState::StartingChat);
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::StartingChat));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::StartingChat));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

	TEST_METHOD(StartChatWithCharacter_WaitForFirstMessage_ExpectStateChangeAndAudioGeneratedWarning)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			TestRunner->SetSuppressLogWarnings(ECQTestSuppressLogBehavior::True);
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});

		/** Test */
		WaitUntilState(VoxtaClientState::WaitingForUserReponse);

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::WaitingForUserReponse));
			ASSERT_THAT(AreEqual(m_messages.Num(), 1));
			ASSERT_THAT(AreEqual(m_messages[0].Value.GetCharId(), m_voxtaId));
			ASSERT_THAT(AreEqual(m_messages[0].Key.GetId(), m_voxtaId));
			ASSERT_THAT(AreEqual(m_messages[0].Key.GetName(), m_voxtaName));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

	TEST_METHOD(StartChatWithCharacter_WaitForFirstMessageAudioPlaybackStart_ExpectStateChange)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_cache_playbackActor = &m_actorTestSpawner->SpawnActor<ATestPlaybackActor>();
			m_cache_playbackActor->Initialize(m_voxtaId);
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});

		/** Test */
		WaitUntilState(VoxtaClientState::AudioPlayback);

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::AudioPlayback));
			ASSERT_THAT(AreEqual(m_messages.Num(), 1));
			ASSERT_THAT(AreEqual(m_messages[0].Value.GetCharId(), m_voxtaId));
			ASSERT_THAT(AreEqual(m_messages[0].Key.GetId(), m_voxtaId));
			ASSERT_THAT(AreEqual(m_messages[0].Key.GetName(), m_voxtaName));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}
#pragma endregion

#pragma region SendUserInput
	TEST_METHOD(SendUserInput_WithInvalidStartingState_ExpectWarningAndStateRemainsUnchanged)
	{
		PRE_TEST;
		FString text = TEXT("some user text");
		TestCommandBuilder.Do([this, text] ()
		{
			/** Setup */
			m_cache_state = m_voxtaClient->GetCurrentState();
			TestRunner->SetSuppressLogErrors();
			m_voxtaClient->SetLogFilter(false);

			/** Test */
			m_voxtaClient->SendUserInput(text);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, m_cache_state));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), m_cache_state));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(text, ELogVerbosity::Type::Warning)));
		});
	}

	TEST_METHOD(SendUserInput_ExpectStateChange)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::ReadyForUserInput);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->SendUserInput(TEXT("some user text"));
		});

		/** Test */
		WaitUntilState(VoxtaClientState::GeneratingReply);

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::GeneratingReply));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetCurrentState(), VoxtaClientState::GeneratingReply));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

	TEST_METHOD(SendUserInput_WaitForReply_ExpectMessageBroadcast)
	{
		PRE_TEST;
		/** Setup */
		FString text = TEXT("Hey there!");
		PreconfigureClient(PreconfigureClientState::ReadyForUserInput);
		TestCommandBuilder.Do([this, text] ()
		{
			m_voxtaClient->SendUserInput(text);
		});

		/** Test */
		WaitUntilState(VoxtaClientState::WaitingForUserReponse);

		TestCommandBuilder.Do([this, text] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::WaitingForUserReponse));
			ASSERT_THAT(AreEqual(m_messages.Num(), 3)); // 1: ai greeting, 2: user message, 3: ai reply
			ASSERT_THAT(AreEqual(m_messages[1].Value.GetTextContent(), text));
			ASSERT_THAT(AreEqual(m_messages[1].Key.GetId(), m_voxtaClient->GetUserId()));
			ASSERT_THAT(IsTrue(m_messages[1].Key.GetId().IsValid()));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}
#pragma endregion

#pragma region NotifyAudioPlaybackComplete
	TEST_METHOD(NotifyAudioPlaybackComplete_ExpectStateChange)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		
		TestCommandBuilder.Do([this] ()
		{
			m_cache_playbackActor = &m_actorTestSpawner->SpawnActor<ATestPlaybackActor>();
			m_cache_playbackActor->m_audioPlaybackComponent->VoxtaMessageAudioPlaybackFinishedEventNative.AddLambda(
			[this] (const FGuid& finishedMessageId)
			{
				m_cache_guid = finishedMessageId;
			});
			m_cache_playbackActor->Initialize(m_voxtaId);
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});

		WaitUntilState(VoxtaClientState::AudioPlayback);
		WaitUntilState(VoxtaClientState::WaitingForUserReponse);

		TestCommandBuilder.Do([this] ()
		{			
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::WaitingForUserReponse));
			ASSERT_THAT(IsTrue(m_cache_guid.IsValid()));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}
#pragma endregion

#pragma region GetServerAddress
	TEST_METHOD(GetServerAddress_WithValidHost_ExpectReturnSameAddress)
	{
		PRE_TEST;
		FString hostAddress = FString(TEXT("127.0.0.1"));
		TestCommandBuilder.Do([this, hostAddress] ()
		{
			/** Setup */
			m_voxtaClient->StartConnection(hostAddress, 5384);

			/** Test */
			FString address = m_voxtaClient->GetServerAddress();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(hostAddress, address));
		});
	}

	TEST_METHOD(GetServerAddress_WithLocalHost_ExpectReturnIpv4)
	{
		PRE_TEST;
		FString hostAddress = FString(TEXT("localhost"));
		TestCommandBuilder.Do([this, hostAddress] ()
		{
			/** Setup */
			m_voxtaClient->StartConnection(hostAddress, 5384);

			/** Test */
			FString address = m_voxtaClient->GetServerAddress();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(FString(TEXT("127.0.0.1")), address));
		});
	}

	TEST_METHOD(GetServerAddress_WithEmptyAddress_ExpectReturnEmpty)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogErrors();
			m_voxtaClient->StartConnection(FString(), 5384);

			/** Test */
			FString address = m_voxtaClient->GetServerAddress();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(FString(), address));
		});
	}

	TEST_METHOD(GetServerAddress_WithInvalidAddress_ExpectReturnEmpty)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogErrors();
			m_voxtaClient->StartConnection(FString("1.2.3.4.5"), 5384);

			/** Test */
			FString address = m_voxtaClient->GetServerAddress();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(FString(), address));
		});
	}

	TEST_METHOD(GetServerAddress_WithInvalidStartingState_ExpectReturnEmpty)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FString address = m_voxtaClient->GetServerAddress();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(FString(), address));
		});
	}
#pragma endregion

#pragma region GetServerPort
	TEST_METHOD(GetServerPort_WithValidHost_ExpectReturnSamePort)
	{
		PRE_TEST;
		int hostPort = 5384;
		TestCommandBuilder.Do([this, hostPort] ()
		{
			/** Setup */
			m_voxtaClient->StartConnection(TEXT("127.0.0.1"), hostPort);

			/** Test */
			int port = m_voxtaClient->GetServerPort();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(hostPort, port));
		});
	}

	TEST_METHOD(GetServerPort_WithEmptyAddress_ExpectReturnMinusOne)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogErrors();
			m_voxtaClient->StartConnection(FString(), 5384);

			/** Test */
			int port = m_voxtaClient->GetServerPort();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(-1, port));
		});
	}

	TEST_METHOD(GetServerPort_WithInvalidAddress_ExpectReturnMinusOne)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogErrors();
			m_voxtaClient->StartConnection(FString("1.2.3.4.5"), 5384);

			/** Test */
			int port = m_voxtaClient->GetServerPort();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(-1, port));
		});
	}

	TEST_METHOD(GetServerPort_WithInvalidStartingState_ExpectReturnEmpty)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			int port = m_voxtaClient->GetServerPort();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(-1, port));
		});
	}
#pragma endregion

#pragma region GetVoiceInputHandler
	TEST_METHOD(GetVoiceInputHandler_ExpectValidPtr)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			UVoxtaAudioInput* ptr = m_voxtaClient->GetVoiceInputHandler();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsNotNull(ptr));
		});
	}
#pragma endregion

#pragma region GetCurrentState
	TEST_METHOD(GetCurrentState_ExpectCorrectReturnsWhileStartingUpAndShuttingDown)
	{
		PRE_TEST;
		/** Setup */
		TArray<VoxtaClientState> expectedStateResponses = {
			VoxtaClientState::AttemptingToConnect,
			VoxtaClientState::Authenticated,
			VoxtaClientState::Idle,
			VoxtaClientState::Terminated
		};

		/** Test */
		m_cache_state = m_voxtaClient->GetCurrentState();
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->Disconnect();
		});

		WaitUntilState(VoxtaClientState::Terminated);
		TestCommandBuilder.Do([this, expectedStateResponses] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_latestNewStateResponse, VoxtaClientState::Terminated));

			ASSERT_THAT(AreEqual(expectedStateResponses.Num(), m_orderedListOfNewStateResponses.Num()));

			for (size_t i = 0; i < m_orderedListOfNewStateResponses.Num(); i++)
			{
				ASSERT_THAT(AreEqual(expectedStateResponses[i], m_orderedListOfNewStateResponses[i]));
			}
		});
	}
#pragma endregion

#pragma region GetUserId
	TEST_METHOD(GetUserId_WithInvalidStartingState_ExpectReturnEmpty)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FGuid userId = m_voxtaClient->GetUserId();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(FGuid(), userId));
		});
	}

	TEST_METHOD(GetUserId_ExpectValidId)
	{
		PRE_TEST;
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FGuid userId = m_voxtaClient->GetUserId();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(userId.IsValid()));
		});
	}
#pragma endregion

#pragma region GetRegisteredAudioPlaybackHandlerForID
	TEST_METHOD(GetRegisteredAudioPlaybackHandlerForID_WithInvalidStartingState_EmptyString_ExpectNullptr)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			const UVoxtaAudioPlayback* playbackHandler;

			/** Test */
			playbackHandler = m_voxtaClient->GetRegisteredAudioPlaybackHandlerForID(FGuid());

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsNull(playbackHandler));
		});
	}

	TEST_METHOD(GetRegisteredAudioPlaybackHandlerForID_WithInvalidStartingState_RandomGuid_ExpectNullptr)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			const UVoxtaAudioPlayback* playbackHandler;

			/** Test */
			playbackHandler = m_voxtaClient->GetRegisteredAudioPlaybackHandlerForID(FGuid::NewGuid());

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsNull(playbackHandler));
		});
	}

	TEST_METHOD(GetRegisteredAudioPlaybackHandlerForID_ValidStartingStateWithoutListener_VoxtaGuid_ExpectNullptr)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			 const UVoxtaAudioPlayback* playbackHandler = m_voxtaClient->GetRegisteredAudioPlaybackHandlerForID(m_voxtaId);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsNull(playbackHandler));
		});
	}

	TEST_METHOD(GetRegisteredAudioPlaybackHandlerForID_ValidStartingState_VoxtaGuid_ExpectListenerPtr)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_cache_playbackActor = &m_actorTestSpawner->SpawnActor<ATestPlaybackActor>();
			m_cache_playbackActor->Initialize(m_voxtaId);
			
			/** Test */
			const UVoxtaAudioPlayback* playbackHandler = m_voxtaClient->GetRegisteredAudioPlaybackHandlerForID(m_voxtaId);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsNotNull(playbackHandler));
		});
	}
#pragma endregion

#pragma region GetChatSession
	TEST_METHOD(GetChatSession_WithInvalidStartingState_ExpectNullptr)
	{
		/** Setup */
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			const FChatSession* chatSession = m_voxtaClient->GetChatSession();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsNull(chatSession));
		});
	}

	TEST_METHOD(GetChatSession_AfterStartingChat_ExpectValidPtr)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::ReadyForUserInput);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			 const FChatSession* chatSession = m_voxtaClient->GetChatSession();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsNotNull(chatSession));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}
#pragma endregion

#pragma region GetA2FHandler
	TEST_METHOD(GetA2FHandler_WithInvalidStartingState_ExpectValidPtr)
	{
		/** Setup */
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			const Audio2FaceRESTHandler* chatSession = m_voxtaClient->GetA2FHandler();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsNotNull(chatSession));
		});
	}
#pragma endregion

#pragma region TryRegisterPlaybackHandler
	TEST_METHOD(TryRegisterPlaybackHandler_WithNullptr_ExpectFalseAndErrorLog)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogErrors();
			FGuid characterId = FGuid::NewGuid();
			bool result = false;

			/** Test */
			result = m_voxtaClient->TryRegisterPlaybackHandler(characterId, nullptr);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsFalse(result));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(characterId, ELogVerbosity::Type::Error)));
		});
	}

	TEST_METHOD(TryRegisterPlaybackHandler_WithDuplicateGuid_ExpectFalseAndWarning)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogWarnings();
			FGuid characterId = FGuid::NewGuid();
			m_cache_playbackActor = &m_actorTestSpawner->SpawnActor<ATestPlaybackActor>();
			m_voxtaClient->TryRegisterPlaybackHandler(characterId,
				m_cache_playbackActor->m_audioPlaybackComponent);
			bool result = false;

			/** Test */
			result = m_voxtaClient->TryRegisterPlaybackHandler(characterId,
				m_cache_playbackActor->m_audioPlaybackComponent);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsFalse(result));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(characterId, ELogVerbosity::Type::Warning)));
		});
	}

	TEST_METHOD(TryRegisterPlaybackHandler_WithRandomGuid_ExpectTrueAndBroadcast)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			FGuid characterId = FGuid::NewGuid();
			m_cache_playbackActor = &m_actorTestSpawner->SpawnActor<ATestPlaybackActor>();
			bool result = false;

			/** Test */
			result = m_voxtaClient->TryRegisterPlaybackHandler(characterId, m_cache_playbackActor->m_audioPlaybackComponent);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(result));
			ASSERT_THAT(AreEqual(1, m_playbackHandlers.Num()));
			ASSERT_THAT(AreEqual(characterId, m_playbackHandlers[0].Value));
			ASSERT_THAT(IsNotNull(m_playbackHandlers[0].Key));
		});
	}

	TEST_METHOD(TryRegisterPlaybackHandler_WithRandomGuid_ExpectTrue)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			FGuid characterId = FGuid::NewGuid();
			m_cache_playbackActor = &m_actorTestSpawner->SpawnActor<ATestPlaybackActor>();
			bool result = false;

			/** Test */
			result = m_voxtaClient->TryRegisterPlaybackHandler(characterId, m_cache_playbackActor->m_audioPlaybackComponent);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(result));
		});
	}
#pragma endregion

#pragma region TryUnregisterPlaybackHandler
	TEST_METHOD(TryUnregisterPlaybackHandler_WithInvalidState_ExpectFalseAndWarning)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogWarnings();
			FGuid characterId = FGuid::NewGuid();
			bool result = false;

			/** Test */
			result = m_voxtaClient->TryUnregisterPlaybackHandler(characterId);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsFalse(result));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(characterId, ELogVerbosity::Type::Warning)));
		});
	}

	TEST_METHOD(UnregisterPlaybackHandler_ExpectTrue)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogWarnings();
			FGuid characterId = FGuid::NewGuid();
			m_cache_playbackActor = &m_actorTestSpawner->SpawnActor<ATestPlaybackActor>();
			m_voxtaClient->TryRegisterPlaybackHandler(characterId, m_cache_playbackActor->m_audioPlaybackComponent);
			bool result = false;

			/** Test */
			result = m_voxtaClient->TryUnregisterPlaybackHandler(characterId);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(result));
		});
	}
#pragma endregion

#pragma region utility
	enum class PreconfigureClientState : uint8
	{
		Started,
		CharacterListLoaded,
		ReadyForUserInput
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
		WaitUntilState(VoxtaClientState::Idle);
		if (state == PreconfigureClientState::CharacterListLoaded)
		{
			return;
		}
		TestCommandBuilder.Do([this] ()
		{
			m_cache_playbackActor = &m_actorTestSpawner->SpawnActor<ATestPlaybackActor>();
			m_cache_playbackActor->Initialize(m_voxtaId);
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});
		WaitUntilState(VoxtaClientState::WaitingForUserReponse);
	}

	void WaitForFixedDuration(double seconds)
	{
		AddCommand(new FWaitUntil(*TestRunner, [this, seconds] ()
		{
			if (m_endTime == FDateTime::MaxValue())
			{
				m_endTime = FDateTime::Now() + FTimespan::FromSeconds(seconds);
			}
			return FDateTime::Now() > m_endTime;
		}, FTimespan::FromSeconds(UNRESPONSIVE_TIMEOUT)));
	}

	void WaitUntilState(VoxtaClientState newState)
	{
		AddCommand(new FWaitUntil(*TestRunner, [&, newState] () 
			{
				return m_voxtaClient->GetCurrentState() == newState; 
			}, FTimespan::FromSeconds(UNRESPONSIVE_TIMEOUT)));
	}
#pragma endregion
};