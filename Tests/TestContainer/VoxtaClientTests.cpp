// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "CQTest.h"
#include "VoxtaClient.h"

/**
 * VoxtaClientTests
 * Tester class that will test the full public API of VoxtaClient.
 *
 * NOTE: These are integration tests and require VoxtaServer to be running on localhost.
 */
TEST_CLASS(VoxtaClientTests, "Game.Voxta")
{
	UGameInstance* gameInstance;
	UVoxtaClient* voxtaClient;

	/** Create a new clean gameInstance and voxtaclient for ever test, to inter-test false positives. */
	BEFORE_EACH()
	{
		gameInstance = NewObject<UGameInstance>();
		gameInstance->Init();
		voxtaClient = gameInstance->GetSubsystem<UVoxtaClient>();
	}

	/** Ensure we don't cause leaks in the Unreal Engine GC */
	AFTER_EACH()
	{
		gameInstance->Shutdown();
		voxtaClient = nullptr;
		gameInstance = nullptr;
	}

	/**
	 * Check if the SubSystem was fetched correctly.
	 * This is mainly just a sanity-check, if this fails something went wrong inside of Unreal or the Plugin configution
	 * inside of the test project.
	 */
	TEST_METHOD(GetVoxtaSubsystem_ExpectNonNull)
	{
		ASSERT_THAT(IsNotNull(voxtaClient));
	}
};