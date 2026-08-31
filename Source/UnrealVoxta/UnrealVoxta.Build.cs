// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

using System.IO;
using UnrealBuildTool;

public class UnrealVoxta : ModuleRules
{
	public UnrealVoxta(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new [] { "Core", "CoreUObject" } );

		PrivateDependencyModuleNames.AddRange(new [] { "Engine" } );
	}
}
