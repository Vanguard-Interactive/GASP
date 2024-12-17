// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GASP : ModuleRules
{
	public GASP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		
		bEnableNonInlinedGenCppWarnings = true;
		
		PublicDependencyModuleNames.AddRange(new string[] { "CoreUObject", "Core", "Engine", "UMG" });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "MotionTrajectory", 
			"PoseSearch", "Chooser", "AnimationWarpingRuntime", "BlendStack", "NetCore" });
	}
}
