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
#include "ChatSession.h"
#include "VoxtaVersionData.h"
#include "Components/ActorTestSpawner.h"
#include "TestGameInstance.h"
#include "TestPlaybackActor.h"
#include "Logging/StructuredLog.h"
#include "LogUtility/Public/Defines.h"

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
	TSharedPtr<TestLogSink> m_testLogSink;
	FActorTestSpawner* m_actorTestSpawner;

	FDelegateHandle m_stateChangedEventHandle;
	FDelegateHandle m_characterRegisteredEventHandle;
	FDelegateHandle m_charMessageAddedEventHandle;
	FDelegateHandle m_charMessageRemovedEventHandle;
	FDelegateHandle m_audioPlaybackRegisteredEventHandle;
	FDelegateHandle m_chatSessionStartedEventNative;
	FDelegateHandle m_chatSessionStoppedEventHandle;

	VoxtaClientState m_latestNewStateResponse;
	TArray<VoxtaClientState> m_orderedListOfNewStateResponses;
	TArray<FAiCharData> m_characters;
	TArray<TTuple<FBaseCharData, FChatMessage>> m_messages;
	TArray<TTuple<const UVoxtaAudioPlayback*, FGuid>> m_playbackHandlers;

	VoxtaClientState m_cache_state;
	ATestPlaybackActor* m_cache_playbackActor;
	FGuid m_cache_guid;
	bool m_cache_bool;

	TArray<FChatSession> m_ongoingChatSessionsFromBroadcast;
	FDateTime m_endTime = FDateTime::MaxValue();

	/** Create a new clean m_gameInstance and voxtaclient for ever test, to avoid inter-test false positives. */
	BEFORE_EACH()
	{
		TestRunner->SuppressedLogCategories.Add(m_signalRLogCategory);

		m_testLogSink = MakeShared<TestLogSink>();
		m_testLogSink->RegisterTestLogSink();
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
		m_charMessageRemovedEventHandle = m_voxtaClient->VoxtaClientCharMessageRemovedEventNative.AddLambda([this] (const FChatMessage& message)
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

		m_ongoingChatSessionsFromBroadcast.Empty();
		m_chatSessionStartedEventNative = m_voxtaClient->VoxtaClientChatSessionStartedEventNative.AddLambda([this] (const FChatSession& chatSession)
		{
			m_ongoingChatSessionsFromBroadcast.Add(chatSession);
		});

		m_chatSessionStoppedEventHandle = m_voxtaClient->VoxtaClientChatSessionStoppedEventNative.AddLambda([this] (const FChatSession& chatSession)
		{
			int index = m_ongoingChatSessionsFromBroadcast.IndexOfByPredicate([&chatSession] (const FChatSession& inItem)
			{
				return inItem.GetChatId() == chatSession.GetChatId();
			});
			m_ongoingChatSessionsFromBroadcast.RemoveAt(index);
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
		m_voxtaClient->VoxtaClientCharMessageRemovedEventNative.Remove(m_charMessageRemovedEventHandle);
		m_voxtaClient->VoxtaClientAudioPlaybackRegisteredEventNative.Remove(m_audioPlaybackRegisteredEventHandle);
		m_voxtaClient->VoxtaClientChatSessionStartedEventNative.Remove(m_chatSessionStartedEventNative);
		m_voxtaClient->VoxtaClientChatSessionStoppedEventNative.Remove(m_chatSessionStoppedEventHandle);

		m_voxtaClient->Disconnect();
		m_voxtaClient = nullptr;
		m_cache_playbackActor = nullptr;
		m_cache_guid.Invalidate();
		m_cache_bool = false;
		delete m_actorTestSpawner;
		m_actorTestSpawner = nullptr;

		m_latestNewStateResponse = VoxtaClientState::Disconnected;
		m_orderedListOfNewStateResponses.Empty();
		m_characters.Empty();
		m_messages.Empty();
		m_playbackHandlers.Empty();

		m_cache_state = VoxtaClientState::Disconnected;

		GLog->Flush();

		TestRunner->SuppressedLogCategories.Remove(m_signalRLogCategory);
	}

#pragma region Validations
	TEST_METHOD(Validate_GetVoxtaSubsystem_ExpectNonNull)
	{
		PRE_TEST;
		ASSERT_THAT(IsNotNull(m_voxtaClient));
	}

	TEST_METHOD(Validate_GetVoxtaSubsystem_ExpectDefaultDisconnectedState)
	{
		PRE_TEST;
		ASSERT_THAT(AreEqual(VoxtaClientState::Disconnected, m_voxtaClient->GetCurrentState()));
	}
#pragma endregion

#pragma region Serverside Validations
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
	TEST_METHOD(StartConnection_WithValidHost_ExpectNewStateAndBroadCast)
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
			ASSERT_THAT(AreEqual(VoxtaClientState::Idle, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(VoxtaClientState::Idle, m_voxtaClient->GetCurrentState()));
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
			ASSERT_THAT(AreEqual(VoxtaClientState::Disconnected, m_latestNewStateResponse));
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
			ASSERT_THAT(AreEqual(VoxtaClientState::Disconnected, m_latestNewStateResponse));
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
			ASSERT_THAT(AreEqual(VoxtaClientState::Disconnected, m_latestNewStateResponse));
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
			TestRunner->SetSuppressLogWarnings(ECQTestSuppressLogBehavior::True);

			/** Test */
			m_voxtaClient->StartConnection(FString("127.0.0.1"), 5384);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("ignoring new connection attempt"), ELogVerbosity::Type::Warning)));
		});
	}

	TEST_METHOD(StartConnection_WaitForIdle_ExpectCharactersRegisteredAndValid)
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
			ASSERT_THAT(AreEqual(VoxtaClientState::Idle, m_latestNewStateResponse));
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
	TEST_METHOD(Disconnect_WithValidState_ExpectTerminatedStateAndBroadcast)
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
			ASSERT_THAT(AreEqual(VoxtaClientState::Terminated, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(VoxtaClientState::Terminated, m_voxtaClient->GetCurrentState()));
		});
	}

	TEST_METHOD(Disconnect_WithoutConnection_ExpectedIgnoredAndWarningLog)
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

	TEST_METHOD(Disconnect_Silent_ExpectStateChangeWithoutBroadcast)
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
			ASSERT_THAT(AreEqual(m_cache_state, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(VoxtaClientState::Terminated, m_voxtaClient->GetCurrentState()));
			ASSERT_THAT(AreNotEqual(m_cache_state, m_voxtaClient->GetCurrentState()));
		});
	}
#pragma endregion

#pragma region StartChatWithCharacter
	TEST_METHOD(StartChatWithCharacter_WithValidStateAndParams_ExpectStateChangeAndBroadcast)
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
			ASSERT_THAT(AreEqual(VoxtaClientState::StartingChat, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(VoxtaClientState::StartingChat, m_voxtaClient->GetCurrentState()));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

	TEST_METHOD(StartChatWithCharacter_WaitForFirstMessage_ExpectValidMessageAndStateChangeBroadcast)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});

		/** Test */
		WaitUntilState(VoxtaClientState::WaitingForUserResponse);

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(VoxtaClientState::WaitingForUserResponse, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(1, m_messages.Num()));
			ASSERT_THAT(AreEqual(m_voxtaId, m_messages[0].Value.GetCharId()));
			ASSERT_THAT(AreEqual(m_voxtaId, m_messages[0].Key.GetId()));
			ASSERT_THAT(AreEqual(m_voxtaName, m_messages[0].Key.GetName()));
			ASSERT_THAT(AreEqual(1, m_ongoingChatSessionsFromBroadcast.Num()));
			ASSERT_THAT(IsTrue(m_ongoingChatSessionsFromBroadcast[0].GetChatId().IsValid()));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetChatSession()->GetChatId(), m_ongoingChatSessionsFromBroadcast[0].GetChatId()));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

	TEST_METHOD(StartChatWithCharacter_WaitForFirstMessage_ExpectAudioPlaybackStartAndStateChange)
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
			ASSERT_THAT(AreEqual(VoxtaClientState::AudioPlayback, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(1, m_messages.Num()));
			ASSERT_THAT(AreEqual(m_voxtaId, m_messages[0].Value.GetCharId()));
			ASSERT_THAT(AreEqual(m_voxtaId, m_messages[0].Key.GetId()));
			ASSERT_THAT(AreEqual(m_voxtaName, m_messages[0].Key.GetName()));
			ASSERT_THAT(AreEqual(1, m_ongoingChatSessionsFromBroadcast.Num()));
			ASSERT_THAT(IsTrue(m_ongoingChatSessionsFromBroadcast[0].GetChatId().IsValid()));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetChatSession()->GetChatId(), m_ongoingChatSessionsFromBroadcast[0].GetChatId()));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

	TEST_METHOD(StartChatWithCharacter_WithContext_ExpectContextSaved)
	{
		PRE_TEST;

		FString context = TEXT("some text context for the conversation");
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this, context] ()
		{
			TestRunner->SetSuppressLogWarnings(ECQTestSuppressLogBehavior::True);
			m_voxtaClient->StartChatWithCharacter(m_voxtaId, context);
		});

		/** Test */
		WaitUntilState(VoxtaClientState::WaitingForUserResponse);

		TestCommandBuilder.Do([this, context] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(VoxtaClientState::WaitingForUserResponse, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(context, m_voxtaClient->GetChatSession()->GetChatContext()));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

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
			ASSERT_THAT(AreEqual(m_cache_state, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(m_cache_state, m_voxtaClient->GetCurrentState()));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(characterId, ELogVerbosity::Type::Error)));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring("Disconnected", ELogVerbosity::Type::Error)));
			ASSERT_THAT(AreEqual(0, m_ongoingChatSessionsFromBroadcast.Num()));

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
			ASSERT_THAT(AreEqual(m_cache_state, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(m_cache_state, m_voxtaClient->GetCurrentState()));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("empty"), ELogVerbosity::Type::Error)));
			ASSERT_THAT(AreEqual(0, m_ongoingChatSessionsFromBroadcast.Num()));

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
			ASSERT_THAT(AreEqual(m_cache_state, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(m_cache_state, m_voxtaClient->GetCurrentState()));
			ASSERT_THAT(AreEqual(0, m_ongoingChatSessionsFromBroadcast.Num()));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(characterId, ELogVerbosity::Type::Error)));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}
#pragma endregion

#pragma region SetGlobalAudioFallbackEnabled
	TEST_METHOD(SetGlobalAudioFallbackEnabled_DisabledByDefault_ExpectNoAudioPlaybackAndLogOfSkipping)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});

		/** Test */
		WaitUntilState(VoxtaClientState::WaitingForUserResponse);

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(VoxtaClientState::WaitingForUserResponse, m_latestNewStateResponse));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("Skipping audio playback of message"), ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsFalse(m_orderedListOfNewStateResponses.ContainsByPredicate([] (VoxtaClientState state) { return state == VoxtaClientState::AudioPlayback; })));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

	TEST_METHOD(SetGlobalAudioFallbackEnabled_SetTrue_ExpectPlaybackStart)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->SetGlobalAudioFallbackEnabled(true);
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});

		/** Test */
		WaitUntilState(VoxtaClientState::AudioPlayback);

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(VoxtaClientState::AudioPlayback, m_latestNewStateResponse));
			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

	TEST_METHOD(SetGlobalAudioFallbackEnabled_EnableThenDisableAgain_ExpectNoAudioPlaybackAndLogOfSkipping)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->SetGlobalAudioFallbackEnabled(true);
			m_voxtaClient->SetGlobalAudioFallbackEnabled(false);
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});

		/** Test */
		WaitUntilState(VoxtaClientState::WaitingForUserResponse);

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(VoxtaClientState::WaitingForUserResponse, m_latestNewStateResponse));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("Skipping audio playback of message"), ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsFalse(m_orderedListOfNewStateResponses.ContainsByPredicate([] (VoxtaClientState state) { return state == VoxtaClientState::AudioPlayback; })));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}
#pragma endregion

#pragma region IsGlobalAudioFallbackActive
	TEST_METHOD(IsGlobalAudioFallbackActive_DisabledByDefault_ExpectFalse)
	{
		PRE_TEST;

		GLog->Flush();
		ASSERT_THAT(IsFalse(m_voxtaClient->IsGlobalAudioFallbackActive()));

		TEST_DELAY;
	}

	TEST_METHOD(IsGlobalAudioFallbackActive_DisabledByDefault_ExpectFalseDuringConversation)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::ReadyForUserInput);
		TestCommandBuilder.Do([this] ()
		{
			GLog->Flush();
			ASSERT_THAT(IsFalse(m_voxtaClient->IsGlobalAudioFallbackActive()));
		});

		TEST_DELAY;
	}

	TEST_METHOD(IsGlobalAudioFallbackActive_AfterSetTrue_ExpectTrueDuringConversation)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->SetGlobalAudioFallbackEnabled(true);
			ASSERT_THAT(IsTrue(m_voxtaClient->IsGlobalAudioFallbackActive()));
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});

		/** Test */
		WaitUntilState(VoxtaClientState::AudioPlayback);

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(m_voxtaClient->IsGlobalAudioFallbackActive()));
			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}

	TEST_METHOD(IsGlobalAudioFallbackActive_EnableThenDisableAgain_ExpectTrueThenFalse)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->SetGlobalAudioFallbackEnabled(true);
			ASSERT_THAT(IsTrue(m_voxtaClient->IsGlobalAudioFallbackActive()));
			m_voxtaClient->SetGlobalAudioFallbackEnabled(false);
			ASSERT_THAT(IsFalse(m_voxtaClient->IsGlobalAudioFallbackActive()));
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});

		/** Test */
		WaitUntilState(VoxtaClientState::WaitingForUserResponse);

		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsFalse(m_voxtaClient->IsGlobalAudioFallbackActive()));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
	}
#pragma endregion

#pragma region StopActiveChat
	TEST_METHOD(StopActiveChat_ExpectStateChangeAndBroadcast)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});

		/** Test */
		WaitUntilState(VoxtaClientState::WaitingForUserResponse);

		TestCommandBuilder.Do([this] ()
		{
			ASSERT_THAT(AreEqual(1, m_ongoingChatSessionsFromBroadcast.Num()));
			m_voxtaClient->StopActiveChat();
		});

		WaitUntilState(VoxtaClientState::Idle);

		TestCommandBuilder.Do([this] ()
		{
			GLog->Flush();
			ASSERT_THAT(AreEqual(0, m_ongoingChatSessionsFromBroadcast.Num()));
			ASSERT_THAT(AreEqual(VoxtaClientState::Idle, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(VoxtaClientState::Idle, m_voxtaClient->GetCurrentState()));
		});

		TEST_DELAY;
	}

	TEST_METHOD(StopActiveChat_InvalidStartState_ExpectNothingHappens)
	{
		PRE_TEST;
		/** Setup */
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;

		TestCommandBuilder.Do([this] ()
		{
			GLog->Flush();
			ASSERT_THAT(AreEqual(0, m_ongoingChatSessionsFromBroadcast.Num()));
			ASSERT_THAT(AreEqual(VoxtaClientState::Disconnected, m_voxtaClient->GetCurrentState()));
		});

		TEST_DELAY;
	}

	TEST_METHOD(StopActiveChat_WithoutActiveChat_ExpectNothingHappens)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			ASSERT_THAT(AreEqual(VoxtaClientState::Idle, m_voxtaClient->GetCurrentState()));
			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;

		TestCommandBuilder.Do([this] ()
		{
			GLog->Flush();
			ASSERT_THAT(AreEqual(0, m_ongoingChatSessionsFromBroadcast.Num()));
			ASSERT_THAT(AreEqual(VoxtaClientState::Idle, m_voxtaClient->GetCurrentState()));
		});

		TEST_DELAY;
	}
#pragma endregion

#pragma region UpdateChatContext
	TEST_METHOD(UpdateChatContext_ExpectStateChangeAndBroadcast)
	{
		PRE_TEST;
		/** Setup */
		FString context = TEXT("First text for context: UpdateChatContext_ExpectStateChangeAndBroadcast");
		FString context2 = TEXT("Second text for context: UpdateChatContext_ExpectStateChangeAndBroadcast");
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->StartChatWithCharacter(m_voxtaId);
		});

		/** Test */
		WaitUntilState(VoxtaClientState::WaitingForUserResponse);

		TestCommandBuilder.Do([this, context] ()
		{
			ASSERT_THAT(AreEqual(FString(), m_voxtaClient->GetChatSession()->GetChatContext()));
			m_voxtaClient->UpdateChatContext(context);
		});

		TEST_DELAY;

		TestCommandBuilder.Do([this, context, context2] ()
		{
			ASSERT_THAT(AreEqual(context, m_voxtaClient->GetChatSession()->GetChatContext()));
			m_voxtaClient->UpdateChatContext(context2);
		});

		TEST_DELAY;

		TestCommandBuilder.Do([this, context2] ()
		{
			GLog->Flush();
			ASSERT_THAT(AreEqual(context2, m_voxtaClient->GetChatSession()->GetChatContext()));
		});

		TEST_DELAY;
	}

	TEST_METHOD(UpdateChatContext_InvalidStartState_ExpectNothingHappensAndErrorLog)
	{
		PRE_TEST;
		/** Setup */
		FString context = TEXT("Some text for context: UpdateChatContext_InvalidStartState_ExpectNothingHappens");
		TestRunner->SetSuppressLogWarnings();
		TestCommandBuilder.Do([this, context] ()
		{
			m_voxtaClient->UpdateChatContext(context);
		});

		TEST_DELAY;

		TestCommandBuilder.Do([this] ()
		{
			GLog->Flush();
			ASSERT_THAT(IsNull(m_voxtaClient->GetChatSession()));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("Cannot update context of chatSession"), ELogVerbosity::Type::Warning)));
		});

		TEST_DELAY;
	}

	TEST_METHOD(UpdateChatContext_WithoutActiveChat_ExpectNothingHappensAndErrorLog)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestRunner->SetSuppressLogWarnings();
		FString context = TEXT("Some text for context: UpdateChatContext_WithoutActiveChat_ExpectNothingHappens");
		TestCommandBuilder.Do([this, context] ()
		{
			m_voxtaClient->UpdateChatContext(context);
		});

		TEST_DELAY;

		TestCommandBuilder.Do([this] ()
		{
			GLog->Flush();
			ASSERT_THAT(IsNull(m_voxtaClient->GetChatSession()));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("Cannot update context of chatSession"), ELogVerbosity::Type::Warning)));
		});

		TEST_DELAY;
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
			m_voxtaClient->SetCensoredLogs(false);

			/** Test */
			m_voxtaClient->SendUserInput(text);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(m_cache_state, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(m_cache_state, m_voxtaClient->GetCurrentState()));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(text, ELogVerbosity::Type::Error)));
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
			ASSERT_THAT(AreEqual(VoxtaClientState::GeneratingReply, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(VoxtaClientState::GeneratingReply, m_voxtaClient->GetCurrentState()));

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
		WaitUntilState(VoxtaClientState::WaitingForUserResponse);

		TestCommandBuilder.Do([this, text] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(VoxtaClientState::WaitingForUserResponse, m_latestNewStateResponse));
			ASSERT_THAT(AreEqual(3, m_messages.Num())); // 1: ai greeting, 2: user message, 3: ai reply
			ASSERT_THAT(AreEqual(text, m_messages[1].Value.GetTextContent()));
			ASSERT_THAT(AreEqual(m_voxtaClient->GetUserId(), m_messages[1].Key.GetId()));
			ASSERT_THAT(IsTrue(m_messages[1].Key.GetId().IsValid()));

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
	TEST_METHOD(GetCurrentState_DuringLifecycle_ExpectCorrectStateTransitionSequence)
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

#pragma region GetBrowserUrlForCharacter
	TEST_METHOD(GetBrowserUrlForCharacter_WithInvalidStartingState_ExpectReturnEmpty)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FString characterUrl = m_voxtaClient->GetBrowserUrlForCharacter(FGuid());

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(FString(), characterUrl));
		});
	}

	TEST_METHOD(GetBrowserUrlForCharacter_WithInvalidGuid_ExpectReturnEmpty)
	{
		PRE_TEST;
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FString characterUrl = m_voxtaClient->GetBrowserUrlForCharacter(FGuid());

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(FString(), characterUrl));
		});
	}

	TEST_METHOD(GetBrowserUrlForCharacter_WithUserGuid_ExpectReturnEmpty)
	{
		PRE_TEST;
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FString characterUrl = m_voxtaClient->GetBrowserUrlForCharacter(m_voxtaClient->GetUserId());

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(FString(), characterUrl));
		});
	}

	TEST_METHOD(GetBrowserUrlForCharacter_WithValidVoxtaId_ExpectValidUrl)
	{
		PRE_TEST;
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FString characterUrl = m_voxtaClient->GetBrowserUrlForCharacter(m_voxtaId);

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(FString(TEXT("http://127.0.0.1:5384/characters/35c74d75-e3e4-44af-9389-faade99cc419")), characterUrl));
		});
	}
#pragma endregion

#pragma region GetMainAssistantId
	TEST_METHOD(GetMainAssistantId_WithInvalidStartingState_ExpectInvalidGuid)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FGuid assistantId = m_voxtaClient->GetMainAssistantId();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(FGuid(), assistantId));
		});
	}

	TEST_METHOD(GetMainAssistantId_ExpectValidId)
	{
		PRE_TEST;
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FGuid assistantId = m_voxtaClient->GetMainAssistantId();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(assistantId.IsValid()));
			ASSERT_THAT(AreEqual(m_voxtaId, assistantId));
		});
	}
#pragma endregion

#pragma region TryFetchAndCacheCharacterThumbnail
	TEST_METHOD(TryFetchAndCacheCharacterThumbnail_WithInvalidStartingState_ExpectErrorAndNoDelegateCall)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->TryFetchAndCacheCharacterThumbnail(FGuid::NewGuid(),
				FDownloadedTextureDelegateNative::CreateLambda([&] (bool success, const UTexture2DDynamic* texture, const FIntVector2& dimensions)
				{
					m_cache_bool = true;
				}));
		});

		TEST_DELAY;
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsFalse(m_cache_bool));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("Cannot fetch thumbnail"), ELogVerbosity::Type::Error)));
		});
	}

	TEST_METHOD(TryFetchAndCacheCharacterThumbnail_WithEmptyGuid_ExpectErrorAndNoDelegateCall)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			PRE_TEST;
			TestCommandBuilder.Do([this] ()
			{
				/** Setup */
				TestRunner->SetSuppressLogErrors();

				/** Test */
				m_voxtaClient->TryFetchAndCacheCharacterThumbnail(FGuid(),
					FDownloadedTextureDelegateNative::CreateLambda([&] (bool success, const UTexture2DDynamic* texture, const FIntVector2& dimensions)
					{
						m_cache_bool = true;
					}));
			});

			TEST_DELAY;
			TestCommandBuilder.Do([this] ()
			{
				/** Assert */
				GLog->Flush();
				ASSERT_THAT(IsFalse(m_cache_bool));
				ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("invalid id"), ELogVerbosity::Type::Error)));
			});
		});
	}

	TEST_METHOD(TryFetchAndCacheCharacterThumbnail_WithInvalidCharacterId_ExpectErrorLog)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Setup */
			TestRunner->SetSuppressLogErrors();

			/** Test */
			m_voxtaClient->TryFetchAndCacheCharacterThumbnail(FGuid::NewGuid(),
				FDownloadedTextureDelegateNative::CreateLambda([&] (bool success, const UTexture2DDynamic* texture, const FIntVector2& dimensions)
				{
					m_cache_bool = true;
				}));
			});

		TEST_DELAY;
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsFalse(m_cache_bool));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("isn't present in the current character list"), ELogVerbosity::Type::Error)));
		});
	}

	TEST_METHOD(TryFetchAndCacheCharacterThumbnail_WithValidCharacterNoThumbnail_ExpectDelegateCalled)
	{
		PRE_TEST;
		/** Setup */
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			m_voxtaClient->TryFetchAndCacheCharacterThumbnail(m_voxtaId,
				FDownloadedTextureDelegateNative::CreateLambda([&] (bool success, const UTexture2DDynamic* texture, const FIntVector2& dimensions)
				{
					m_cache_bool = true;
					ASSERT_THAT(IsTrue(success));
					ASSERT_THAT(IsNotNull(texture));
					ASSERT_THAT(AreEqual(384, dimensions.X));
					ASSERT_THAT(AreEqual(512, dimensions.Y));
				}));
			});

		TEST_DELAY;
		TestCommandBuilder.Do([this] ()
		{
			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(m_cache_bool));
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

#pragma region GetAvailableAiCharactersCopy
	TEST_METHOD(GetAvailableAiCharactersCopy_WithInvalidStartingState_ExpectEmptyArray)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			TArray<FAiCharData> aiCharacters = m_voxtaClient->GetAvailableAiCharactersCopy();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(0, aiCharacters.Num()));
		});
	}

	TEST_METHOD(GetAvailableAiCharactersCopy_ExpectValidArrayIncludingDefaultCharacterExcludingUser)
	{
		PRE_TEST;
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			TArray<FAiCharData> aiCharacters = m_voxtaClient->GetAvailableAiCharactersCopy();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(aiCharacters.Num() > 0));
			bool foundDefault = false;
			bool foundUser = false;
			for (size_t i = 0; i < aiCharacters.Num(); i++)
			{
				foundDefault = foundDefault || aiCharacters[i].GetId() == m_voxtaId;
				foundUser = foundUser || aiCharacters[i].GetId() == m_voxtaClient->GetUserId();
			}
			ASSERT_THAT(IsTrue(foundDefault));
			ASSERT_THAT(IsFalse(foundUser));
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

#pragma region GetChatSessionCopy
	TEST_METHOD(GetChatSessionCopy_WithInvalidStartingState_ExpectDefault)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FChatSession chatSession = m_voxtaClient->GetChatSessionCopy();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsFalse(chatSession.GetChatId().IsValid()));
		});
	}

	TEST_METHOD(GetChatSessionCopy_ExpectSessionWithValidIdAndCharacter)
	{
		PRE_TEST;
		PreconfigureClient(PreconfigureClientState::ReadyForUserInput);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FChatSession chatSession = m_voxtaClient->GetChatSessionCopy();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(chatSession.GetChatId().IsValid()));
			ASSERT_THAT(IsTrue(chatSession.GetCharacterIds().Num() > 0));
			ASSERT_THAT(IsTrue(chatSession.GetCharacterIds()[0] == m_voxtaId));
		});
	}
#pragma endregion

#pragma region GetServerVersionCopy
	TEST_METHOD(GetServerVersionCopy_WithInvalidStartingState_ExpectDefaultAndNonMatchingVersions)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FVoxtaVersionData versionData = m_voxtaClient->GetServerVersionCopy();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(AreEqual(FString(TEXT("not connected")), versionData.GetServerVersion()));
			ASSERT_THAT(IsFalse(versionData.IsMatchingServerVersion()));
			ASSERT_THAT(IsFalse(versionData.IsMatchingAPIVersion()));
		});
	}

	TEST_METHOD(GetServerVersionCopy_WithValidConnection_ExpectMatchingVersionInfo)
	{
		PRE_TEST;
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FVoxtaVersionData versionData = m_voxtaClient->GetServerVersionCopy();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(versionData.IsMatchingServerVersion()));
			ASSERT_THAT(IsTrue(versionData.IsMatchingAPIVersion()));
		});
	}
#pragma endregion

#pragma region IsMatchingAPIVersion
	TEST_METHOD(IsMatchingAPIVersion_WithInvalidStartingState_ExpectFalse)
	{
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			bool isMatchingVersion = m_voxtaClient->IsMatchingAPIVersion();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsFalse(isMatchingVersion));
		});
	}

	TEST_METHOD(IsMatchingAPIVersion_WithValidConnection_ExpectVersionMatch)
	{
		PRE_TEST;
		PreconfigureClient(PreconfigureClientState::CharacterListLoaded);
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			bool isMatchingVersion = m_voxtaClient->IsMatchingAPIVersion();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(isMatchingVersion));
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
			TWeakPtr<Audio2FaceRESTHandler> handler = m_voxtaClient->GetA2FHandler();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsNotNull(handler));
		});
	}
#pragma endregion

#pragma region SetCensoredLogs
	TEST_METHOD(Validate_VoxtaLogCensor_ExpectSensitiveLogParamsCensoredByDefault)
	{
		PRE_TEST;

		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			FString test02 = TEXT("test02");
			SENSITIVE_LOG_BASIC(VoxtaLog, Log, TEXT("test01: %s"), *test02);

			FString test04 = TEXT("test04");
			SENSITIVE_LOG1(VoxtaLog, Log, "test03: {0}", test04);

			FString test06 = TEXT("test06");
			FString test07 = TEXT("test07");
			SENSITIVE_LOG2(VoxtaLog, Log, "test05: {0}, {1}", test06, test07);

			FString test09 = TEXT("test09");
			FString test10 = TEXT("test10");
			FString test11 = TEXT("test11");
			SENSITIVE_LOG3(VoxtaLog, Log, "test08: {0}, {1}, {2}", test09, test10, test11);

			/** Assert */
			GLog->Flush();

			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("test01"), ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsFalse(m_testLogSink->ContainsLogMessageWithSubstring(test02, ELogVerbosity::Type::Log)));

			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("test03"), ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsFalse(m_testLogSink->ContainsLogMessageWithSubstring(test04, ELogVerbosity::Type::Log)));

			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("test05"), ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsFalse(m_testLogSink->ContainsLogMessageWithSubstring(test06, ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsFalse(m_testLogSink->ContainsLogMessageWithSubstring(test07, ELogVerbosity::Type::Log)));

			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("test08"), ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsFalse(m_testLogSink->ContainsLogMessageWithSubstring(test09, ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsFalse(m_testLogSink->ContainsLogMessageWithSubstring(test10, ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsFalse(m_testLogSink->ContainsLogMessageWithSubstring(test11, ELogVerbosity::Type::Log)));
		});
	}

	TEST_METHOD(SetCensoredLogs_False_ExpectSensitiveLogParamsUncensored)
	{
		PRE_TEST;

		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->SetCensoredLogs(false);

			ASSERT_THAT(IsFalse(m_voxtaClient->IsLogCensorActive()));
			ASSERT_THAT(IsFalse(SensitiveLogging::isSensitiveLogsCensored));

			/** Test */
			FString test02 = TEXT("test02");
			SENSITIVE_LOG_BASIC(VoxtaLog, Log, TEXT("test01: %s"), *test02);

			FString test04 = TEXT("test04");
			SENSITIVE_LOG1(VoxtaLog, Log, "test03: {0}", test04);

			FString test06 = TEXT("test06");
			FString test07 = TEXT("test07");
			SENSITIVE_LOG2(VoxtaLog, Log, "test05: {0}, {1}", test06, test07);

			FString test09 = TEXT("test09");
			FString test10 = TEXT("test10");
			FString test11 = TEXT("test11");
			SENSITIVE_LOG3(VoxtaLog, Log, "test08: {0}, {1}, {2}", test09, test10, test11);

			/** Assert */
			GLog->Flush();

			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("test01"), ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(test02, ELogVerbosity::Type::Log)));

			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("test03"), ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(test04, ELogVerbosity::Type::Log)));

			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("test05"), ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(test06, ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(test07, ELogVerbosity::Type::Log)));

			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(TEXT("test08"), ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(test09, ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(test10, ELogVerbosity::Type::Log)));
			ASSERT_THAT(IsTrue(m_testLogSink->ContainsLogMessageWithSubstring(test11, ELogVerbosity::Type::Log)));
		});
	}
#pragma endregion

#pragma region IsLogCensorActive
	TEST_METHOD(IsLogCensorActive_ExpectTrueByDefault)
	{
		/** Setup */
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			/** Test */
			bool isActive = m_voxtaClient->IsLogCensorActive();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsTrue(isActive));
		});
	}

	TEST_METHOD(IsLogCensorActive_SetFalse_ExpectFalse)
	{
		/** Setup */
		PRE_TEST;
		TestCommandBuilder.Do([this] ()
		{
			m_voxtaClient->SetCensoredLogs(false);

			/** Test */
			bool isActive = m_voxtaClient->IsLogCensorActive();

			/** Assert */
			GLog->Flush();
			ASSERT_THAT(IsFalse(isActive));
		});
	}
#pragma endregion

#pragma region Other
	TEST_METHOD(NotifyAudioPlaybackComplete_DuringPlayback_ExpectTransitionToWaitingState)
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
		WaitUntilState(VoxtaClientState::WaitingForUserResponse);

		TestCommandBuilder.Do([this] ()
		{
			GLog->Flush();
			ASSERT_THAT(AreEqual(VoxtaClientState::WaitingForUserResponse, m_latestNewStateResponse));
			ASSERT_THAT(IsTrue(m_cache_guid.IsValid()));

			m_voxtaClient->StopActiveChat();
		});

		TEST_DELAY;
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
			m_voxtaClient->StartConnection(FString(TEXT("127.0.0.1")), 5384);
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
		WaitUntilState(VoxtaClientState::WaitingForUserResponse);
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