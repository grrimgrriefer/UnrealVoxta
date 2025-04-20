// MIT License; Copyright(c) 2020 - 2022 Frozen Storm Interactive, Yoann Potinet

using System.IO;
using UnrealBuildTool;

public class SignalR : ModuleRules
{
	public SignalR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;		

		PrivateIncludePaths.AddRange(new [] {
			Path.Combine(ModuleDirectory, "Public"),
		});

		PrivateDependencyModuleNames.AddRange(new [] { "Core", "CoreUObject", "Engine", "Json", "HTTP", "WebSockets" });
	}
}