// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

public class UnrealVoxtaEditor : ModuleRules
{
	public UnrealVoxtaEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "VoxtaData", "VoxtaUtility_A2F"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore", "AnimGraph", "BlueprintGraph"
			}
			);
	}
}