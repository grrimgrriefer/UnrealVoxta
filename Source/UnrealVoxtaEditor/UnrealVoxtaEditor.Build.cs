// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

/// <summary>
/// Specifies all the requirements to compile the Editor-specific code for the plugin.
/// This is currently limited to a one custom A2F AnimatorBlueprint node.
/// </summary>
public class UnrealVoxtaEditor : ModuleRules
{
	/// <summary>
	/// Constructor.
	/// </summary>
	public UnrealVoxtaEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new [] { "Core", "CoreUObject", "VoxtaData", "VoxtaUtility_A2F" } );

		PrivateDependencyModuleNames.AddRange(new [] { "Engine", "AnimGraph", "BlueprintGraph" } );
	}
}