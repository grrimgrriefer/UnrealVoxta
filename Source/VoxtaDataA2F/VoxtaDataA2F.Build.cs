// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

public class VoxtaDataA2F : ModuleRules
{
	public VoxtaDataA2F(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "Json", "InputCore", "VoxtaData", "HTTP" });
	}
}