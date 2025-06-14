// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

/// <summary>
/// Specifies all the requirements to compile the Data module for the plugin.
/// This contains both readonly- and runtime-data structures.
/// </summary>
public class VoxtaData : ModuleRules
{
	/// <summary>
	/// Constructor.
	/// </summary>
	public VoxtaData(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new [] { "Core", "CoreUObject" });
	}
}