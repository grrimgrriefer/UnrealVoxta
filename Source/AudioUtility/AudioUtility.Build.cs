// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class AudioUtility : ModuleRules
{
	public AudioUtility(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { "MetasoundEngine",	"MetasoundFrontend" });
		
		bool bEnableCaptureInputSupport = true;
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Core",
				"AudioPlatformConfiguration"
			}
		);

		if (Target.Version.MajorVersion >= 5 && Target.Version.MinorVersion >= 1)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"SignalProcessing"
				}
			);
		}

		if (Target.Version.MajorVersion >= 5 && Target.Version.MinorVersion >= 2)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"AudioExtensions"
				}
			);
		}

		if (bEnableCaptureInputSupport)
		{
			if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) ||
			    Target.Platform == UnrealTargetPlatform.Mac)
			{
				PrivateDependencyModuleNames.Add("AudioCaptureRtAudio");
			}
			else if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				PrivateDependencyModuleNames.Add("AudioCaptureAudioUnit");
				PrivateDependencyModuleNames.Add("Core");
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

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"AudioMixer",
					"AudioCaptureCore"
				}
			);
		}
    }
}