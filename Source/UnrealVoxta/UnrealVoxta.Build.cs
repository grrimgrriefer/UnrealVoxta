// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using System.IO;
using UnrealBuildTool;

/// <summary>
/// Specifies all the requirements to compile the main API for all Voxta-related functionality.
/// This module handles state-tracking of the internal VoxtaClient, and request & response handing.
/// </summary>
public class UnrealVoxta : ModuleRules
{
	/// <summary>
	/// Constructor.
	/// </summary>
	public UnrealVoxta(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseRTTI = true;

		PrivateIncludePaths.AddRange(new [] {
			Path.Combine(ModuleDirectory, "Public", "UtilityNodes"),
			Path.Combine(ModuleDirectory, "Private", "Internals")
		});

		PublicDependencyModuleNames.AddRange(new [] { "Core", "CoreUObject", "VoxtaAudioUtility" } );

		PrivateDependencyModuleNames.AddRange(new [] { "Engine", "VoxtaData", "VoxtaUtility_A2F", "SignalR", "HTTP", "Voice", "Json", "Projects", "Networking", "ImageWrapper", "RenderCore" } );

		if (UnrealVoxta.HasModule(this, "OVRLipSync"))
		{
			PublicDependencyModuleNames.Add("OVRLipSync");
			PrivateDependencyModuleNames.Add("VoxtaUtility_OVR");
			PublicDefinitions.Add("WITH_OVRLIPSYNC=1");
		}
		else
		{
			PublicDefinitions.Add("WITH_OVRLIPSYNC=0");
		}
	}

	/// <summary>
	/// Utility function to compile with/without OVR integration, depending on whether the OVR plugin is present & active.
	/// </summary>
	public static bool HasModule(ModuleRules rulesRef, string moduleName)
	{
		try
		{
			return !string.IsNullOrWhiteSpace(rulesRef.GetModuleDirectory(moduleName));
		}
		catch (BuildException)
		{
			return false;
		}
	}
}