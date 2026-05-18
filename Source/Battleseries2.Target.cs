// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class Battleseries2Target : TargetRules
{
	public Battleseries2Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;

        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        BuildEnvironment = TargetBuildEnvironment.Shared;

        bOverrideBuildEnvironment = true;

        ExtraModuleNames.AddRange( new string[] { "Battleseries2" } );
	}
}
