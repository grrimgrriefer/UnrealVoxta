// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "ButtonWithParameter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FClickedWithParam, FString, value);

UCLASS()
class TALKTOMECPPUE_API UButtonWithParameter : public UButton
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FClickedWithParam OnClickedWithParam;

	void Initialize(FString value);

	UFUNCTION()
	void OnClick();

private:
	FString m_parameterValue;
};