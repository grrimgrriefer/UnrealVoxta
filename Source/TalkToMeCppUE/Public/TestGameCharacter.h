// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Voxta/Public/VoxtaClient.h"
#include "TalkToMeCppUeHUD.h"
#include "VoxtaAudioPlayback.h"
#include "TestGameCharacter.generated.h"

/// <summary>
/// Most basic pawn setup for debugging purposes.
/// Has ownership of the VoxtaClient, managing it's lifecycle and it's bindings to the engine and hud.
/// </summary>
UCLASS()
class TALKTOMECPPUE_API ATestGameCharacter : public APawn
{
	GENERATED_BODY()

public:
	/// <summary>
	/// Default constructor, adds the of the UCameraComponent and UVoxtaClient components.
	/// </summary>
	ATestGameCharacter();

	/// <summary>
	/// Starts the VoxtaClient connection & sets up the binding with the HUD.
	/// </summary>
	void StartVoxtaClient();

private:
	UCameraComponent* m_camera;
	UVoxtaClient* m_voxtaClient;
	ATalkToMeCppUeHUD* m_hud;
	UVoxtaAudioPlayback* m_audioPlaybackHandler;

	UCameraComponent* deleteme;

	/// <summary>
	/// Connects the events between the UVoxtaClient and the ATalkToMeCppUeHUD.
	/// </summary>
	/// <returns>True if connection was established.</returns>
	bool TryConnectToHud();

	/// <summary>
	/// Cleans up the bindings between the UVoxtaClient and the ATalkToMeCppUeHUD.
	/// </summary>
	/// <returns>True if connection was established.</returns>
	bool TryDisconnectToHud();

	UFUNCTION()
	void VoxtaClientStateChanged(VoxtaClientState newState);

	///~ Begin APawn overrides.
protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	///~ End APawn overrides.
};
