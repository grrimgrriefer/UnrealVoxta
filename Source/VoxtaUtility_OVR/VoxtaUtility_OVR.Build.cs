// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

/// <summary>
/// Specifies all the requirements to compile the OVR specific code of the plugin.
/// </summary>
public class VoxtaUtility_OVR : ModuleRules
{
	/// <summary>
	/// Constructor. 
	/// </summary>
	public VoxtaUtility_OVR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new [] { "Core", "CoreUObject", "VoxtaData", "OVRLipSync" });
	}
}