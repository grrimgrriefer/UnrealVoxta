// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

public class VoxtaUtility : ModuleRules
{
	public VoxtaUtility(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AudioUtility", "VoxtaData", "SignalR", "HTTP", "Voice" });

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
			PublicDependencyModuleNames.Add("VoxtaDataOVR");
			PublicDefinitions.Add("WITH_OVRLIPSYNC=1");
		}
		else
		{
			PublicDefinitions.Add("WITH_OVRLIPSYNC=0");
		}
	}
}