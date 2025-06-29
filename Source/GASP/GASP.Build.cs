// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GASP : ModuleRules
{
	public GASP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Warning;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"NetCore",
			"GameplayTags",
			"MotionWarping",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"MotionTrajectory",
			"PoseSearch",
			"Chooser",
			"AnimationWarpingRuntime",
			"BlendStack",
			"PhysicsCore",
			"AnimGraphRuntime",
		});

		if (Target.Type == TargetRules.TargetType.Editor)
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"GameplayDebugger"
			});
	}
}