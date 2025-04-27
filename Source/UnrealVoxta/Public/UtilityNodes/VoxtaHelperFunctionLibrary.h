// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxtaHelperFunctionLibrary.generated.h"

/**
 * Library of functionality exposed to blueprints.
 * Mainly used for simple C++ utility that would require a ton of BP nodes to emulate.
 */
UCLASS(Category = "Voxta")
class UNREALVOXTA_API UVoxtaHelperFunctionLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#pragma region public API
public:
	/**
	 * Check if the given string is a valid IPv4 address (or localhost)
	 * 
	 * @param address The string to check.
	 * 
	 * @return True if the given string is a valid IPv4 (or localhost), false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	static bool IsIpv4Valid(const FString& address);

	/**
	 * Converts a float to a string value with a given amount of precision.
	 * 
	 * @param input The raw float value to convert.
	 * @param digitsAfterPoint The number of digits after point it should use before rounding the value.
	 * 
	 * @return The converted string value
	 */
	UFUNCTION(BlueprintPure, Category = "Voxta")
	static FString FloatToStringPrecision(float input, int digitsAfterPoint = 2);
#pragma endregion

#pragma region data
	static inline const FString LOCALHOST = TEXT("localhost");
#pragma endregion
};