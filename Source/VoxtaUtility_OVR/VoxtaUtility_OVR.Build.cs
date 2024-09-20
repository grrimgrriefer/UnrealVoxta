// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

public class VoxtaUtility_OVR : ModuleRules
{
	public VoxtaUtility_OVR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "VoxtaData", "OVRLipSync", "VoxtaUtility_OVR" });
	}
}