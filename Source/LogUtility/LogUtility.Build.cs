// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

public class LogUtility : ModuleRules
{
	public LogUtility(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
	}
}