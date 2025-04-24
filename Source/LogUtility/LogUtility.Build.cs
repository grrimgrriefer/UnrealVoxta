// Copyright(c) 2025 grrimgrriefer & DZnnah, see LICENSE for details.

using System.IO;
using UnrealBuildTool;

/// <summary>
/// Specifies all the requirements to compile the LogUtility (mostly mactros and other defines)
/// Should not depend on any module.
/// </summary>
public class LogUtility : ModuleRules
{
	/// <summary>
	/// Constructor. 
	/// </summary>
	public LogUtility(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new [] { "Core" });
	}
}