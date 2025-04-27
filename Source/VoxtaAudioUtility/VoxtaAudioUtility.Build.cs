// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

using UnrealBuildTool;
using System.IO;

/// <summary>
/// Specifies all the requirements to compile the Audio specific code of the plugin.
/// Encapsulates microphone input, audio playback, and WAV file decoding.
/// </summary>
public class VoxtaAudioUtility : ModuleRules
{
	/// <summary>
	/// Constructor. 
	/// </summary>
	public VoxtaAudioUtility(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(new [] {
			Path.Combine(ModuleDirectory, "Public", "RuntimeAudioImporter"),
		});

		PublicDependencyModuleNames.AddRange(new [] { "Core", "CoreUObject", "WebSockets", "AudioCaptureCore" });

		PrivateDependencyModuleNames.AddRange(new [] { "Engine", "Voice", "VoxtaData", "AudioPlatformConfiguration", "AudioExtensions" });

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) ||
			Target.Platform == UnrealTargetPlatform.Mac)
		{
			PrivateDependencyModuleNames.Add("AudioCaptureRtAudio");
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PrivateDependencyModuleNames.Add("AudioCaptureAudioUnit");
			PublicFrameworks.AddRange(new [] { "CoreAudio", "AVFoundation", "AudioToolbox" });
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.AddRange(
				new []
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