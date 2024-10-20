// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

public class UnrealVoxta : ModuleRules
{
	public UnrealVoxta(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseRTTI = true;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "VoxtaAudioUtility", "VoxtaData", "SignalR", "HTTP", "Voice", "VoxtaUtility_A2F"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore", "Json", "JsonUtilities", "Projects", "Networking"
			}
			);

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
			PrivateDependencyModuleNames.Add("VoxtaUtility_OVR");
			PublicDependencyModuleNames.Add("OVRLipSync");
			PublicDefinitions.Add("WITH_OVRLIPSYNC=1");
		}
		else
		{
			PublicDefinitions.Add("WITH_OVRLIPSYNC=0");
		}
	}
}