// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

public class VoxtaUtility : ModuleRules
{
	public VoxtaUtility(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AudioUtility", "VoxtaData", "SignalR", "HTTP", "Voice" });

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}