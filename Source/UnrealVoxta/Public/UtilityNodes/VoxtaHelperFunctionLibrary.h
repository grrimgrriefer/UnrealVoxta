// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxtaDefines.h"
#include "VoxtaHelperFunctionLibrary.generated.h"

class UVoxtaClient;

UCLASS(Category = "Voxta")
class UNREALVOXTA_API UVoxtaHelperFunctionLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#pragma region public API
public:
	UFUNCTION(BlueprintPure, Category = "Voxta")
	static bool IsIpv4Valid(const FString& address);

	UFUNCTION(BlueprintPure, Category = "Voxta")
	static FString FloatToStringPrecision(float input, int digitsAfterPoint = 2);
#pragma endregion

#pragma region data
	static inline const FString LOCALHOST = TEXT("localhost");
#pragma endregion
};