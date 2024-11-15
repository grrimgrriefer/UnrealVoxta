// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details. Copyright Epic Games, Inc. All
// Rights Reserved.

using UnrealBuildTool;

public class SignalR : ModuleRules
{
	public SignalR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "ApplicationCore", "Json" });

		PrivateDependencyModuleNames.AddRange(new string[] { "HTTP", "WebSockets", "Engine" });
	}
}