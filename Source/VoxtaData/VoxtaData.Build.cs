// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details. Copyright Epic Games, Inc. All
// Rights Reserved.

using UnrealBuildTool;

public class VoxtaData : ModuleRules
{
	public VoxtaData(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "VoxtaData" });

		// There has to be a cleaner way to do this lmao
		bool projectHasOvrLipSync = false;
		try
		{
			projectHasOvrLipSync = !string.IsNullOrWhiteSpace(GetModuleDirectory("OVRLipSync"));
		}
		catch (BuildException)
		{
			projectHasOvrLipSync = false;
		}

		if (projectHasOvrLipSync)
		{
			PublicDependencyModuleNames.Add("OVRLipSync");
			PublicDependencyModuleNames.Add("VoxtaUtility_OVR");
			PublicDefinitions.Add("WITH_OVRLIPSYNC=1");
		}
		else
		{
			PublicDefinitions.Add("WITH_OVRLIPSYNC=0");
		}
	}
}