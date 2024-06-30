// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AudioUtility : ModuleRules
{
	public AudioUtility(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AudioUtility" });

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}