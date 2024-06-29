// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "ButtonWithParameter.generated.h"

/// <summary>
/// Custom button class that can be given a FString value which is then returned via the
/// ClickedWithParamEvent when clicked by the user.
/// </summary>
UCLASS()
class TALKTOMECPPUE_API UButtonWithParameter : public UButton
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FClickedWithParamEventCallback, FString, value);

	/// <summary>
	/// Event that will be invoked when the user presses the button.
	/// </summary>
	UPROPERTY()
	FClickedWithParamEventCallback ClickedWithParamEvent;

	/// <summary>
	/// Provide a value that will be returned via the ClickedWithParamEvent event when clicked
	/// by the user.
	/// </summary>
	/// <param name="value">The value to be stored and returned through the event.</param>
	void Initialize(FStringView value);

private:
	FString m_parameterValue;

	/// <summary>
	/// Automatically invoked by the UButton::OnClicked, which then triggers the ClickedWithParamEvent
	/// with the stored FString value provided whith the initialization
	/// </summary>
	UFUNCTION()
	void OnClickInternal();

	///~ Begin UWidget overrides.
public:
	virtual void BeginDestroy() override;
	///~ End UWidget overrides.
};