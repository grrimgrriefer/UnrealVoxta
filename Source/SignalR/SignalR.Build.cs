// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SignalR : ModuleRules
{
	public SignalR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "ApplicationCore", "Json", "SignalR" });

		PrivateDependencyModuleNames.AddRange(new string[] { "HTTP", "WebSockets", "Engine" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
