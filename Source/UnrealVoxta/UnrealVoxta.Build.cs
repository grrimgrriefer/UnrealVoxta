// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

public class UnrealVoxta : ModuleRules
{
	public UnrealVoxta(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(["Core", "CoreUObject", "StateTreeModule"]);

		PrivateDependencyModuleNames.AddRange(["Engine"]);
	}
}
