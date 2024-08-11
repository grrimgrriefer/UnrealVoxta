// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;

public class UnrealVoxta : ModuleRules
{
	public UnrealVoxta(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "AudioUtility", "VoxtaData", "VoxtaUtility", "SignalR", "HTTP", "Voice"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore",
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
			PublicDependencyModuleNames.Add("OVRLipSync");
			PublicDefinitions.Add("WITH_OVRLIPSYNC=1");
		}
		else
		{
			PublicDefinitions.Add("WITH_OVRLIPSYNC=0");
		}
	}
}