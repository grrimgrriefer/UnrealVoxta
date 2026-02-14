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

		PublicDependencyModuleNames.AddRange(new [] { "Core", "CoreUObject", "SignalR" });
		
		PublicSystemIncludePaths.AddRange(new [] { "/home/overlord/.bin/Linux_Unreal_Engine_5.7.2/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v26_clang-20.1.8-rockylinux8/x86_64-unknown-linux-gnu/include/c++/v1" });
	}
}