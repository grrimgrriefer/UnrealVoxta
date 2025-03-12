// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxtaHelperFunctionLibrary.generated.h"

class UVoxtaClient;

UCLASS(Category = "Voxta")
class UNREALVOXTA_API UVoxtaHelperFunctionLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#pragma region public API
public:
	UFUNCTION(BlueprintCallable, Category = "Voxta")
	static bool IsIpv4Valid(const FString& address);
#pragma endregion

#pragma region data
	static inline const FString LOCALHOST = TEXT("localhost");
#pragma endregion
};