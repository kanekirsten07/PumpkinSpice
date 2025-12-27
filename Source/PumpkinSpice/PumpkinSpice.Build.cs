// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PumpkinSpice : ModuleRules
{
	public PumpkinSpice(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "GameplayAbilities", "GameplayTags", "GameplayTasks", "InputCore", "EnhancedInput", "OnlineSubsystemSteam", "OnlineSubsystem", "UMG", "Slate", "SlateCore" });
	}
}
