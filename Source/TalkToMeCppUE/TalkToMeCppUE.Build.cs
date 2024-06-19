// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TalkToMeCppUE : ModuleRules
{
	public TalkToMeCppUE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}