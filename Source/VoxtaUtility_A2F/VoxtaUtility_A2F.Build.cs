// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

/// <summary>
/// Specifies all the requirements to compile the Nvidia Audio2Face specific code of the plugin.
/// </summary>
public class VoxtaUtility_A2F : ModuleRules
{
	/// <summary>
	/// Constructor.
	/// </summary>
	public VoxtaUtility_A2F(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new [] { "Core", "CoreUObject", "Engine" });

		PrivateDependencyModuleNames.AddRange(new [] { "Json", "VoxtaData", "HTTP", "Projects" });
	}
}