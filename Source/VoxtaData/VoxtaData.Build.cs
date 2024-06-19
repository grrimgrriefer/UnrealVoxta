// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VoxtaData : ModuleRules
{
	public VoxtaData(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "VoxtaData" });

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}