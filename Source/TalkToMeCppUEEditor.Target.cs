// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TalkToMeCppUEEditorTarget : TargetRules
{
	public TalkToMeCppUEEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
		ExtraModuleNames.Add("TalkToMeCppUE");
		ExtraModuleNames.Add("Voxta");
		ExtraModuleNames.Add("VoxtaData");
		ExtraModuleNames.Add("SignalR");
	}
}
