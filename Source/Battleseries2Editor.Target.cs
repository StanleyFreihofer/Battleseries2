// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class Battleseries2EditorTarget : TargetRules
{
	public Battleseries2EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;

        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        BuildEnvironment = TargetBuildEnvironment.Shared;

        bOverrideBuildEnvironment = true;

        ExtraModuleNames.AddRange( new string[] { "Battleseries2" } );
	}
}
