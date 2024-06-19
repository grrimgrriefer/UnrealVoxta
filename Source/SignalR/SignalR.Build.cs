// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SignalR : ModuleRules
{
	public SignalR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "ApplicationCore", "Json", "SignalR" });

		PrivateDependencyModuleNames.AddRange(new string[] { "HTTP", "WebSockets", "Engine" });
	}
}