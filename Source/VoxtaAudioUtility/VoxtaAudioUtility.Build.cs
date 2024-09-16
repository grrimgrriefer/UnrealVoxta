// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class VoxtaAudioUtility : ModuleRules
{
	public VoxtaAudioUtility(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "ApplicationCore", "InputCore", "WebSockets", "AudioMixer", "AudioCaptureCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Voice", "AudioPlatformConfiguration", "SignalProcessing", "AudioExtensions" });
		
		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) ||
			Target.Platform == UnrealTargetPlatform.Mac)
		{
			PrivateDependencyModuleNames.Add("AudioCaptureRtAudio");
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PrivateDependencyModuleNames.Add("AudioCaptureAudioUnit");
			PrivateDependencyModuleNames.Add("AudioCaptureCore");
			PublicFrameworks.AddRange(new string[] { "CoreAudio", "AVFoundation", "AudioToolbox" });
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"AudioCaptureAndroid",
					"AndroidPermission"
				}
			);

			string BuildPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(BuildPath, "RuntimeAudioImporter_AndroidAPL.xml"));
		}		
    }
}