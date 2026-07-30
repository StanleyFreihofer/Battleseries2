// Fill out your copyright notice in the Description page of Project Settings.
using UnrealBuildTool;

public class Battleseries2 : ModuleRules
{
	public Battleseries2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                ModuleDirectory,
                System.IO.Path.Combine(ModuleDirectory, "Public")
            }
        );

        PublicDependencyModuleNames.AddRange(
			new string[] 
			{ 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore",
				"ChaosVehicles",
				"ChaosVehiclesEngine",
				"EnhancedInput",
				"MetasoundEngine",
				"Niagara",
				"UMG",
				"Slate",
				"SlateCore",
				"Renderer",
				"RenderCore"
			});

		PrivateDependencyModuleNames.AddRange(
			new string[] 
			{
				//"AssetTools",
				//"UnrealEd"
			});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
