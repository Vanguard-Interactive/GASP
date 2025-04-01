// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/EnumTypes.h"
#include "GameplayTags.h"
#include "Utils/GASPMath.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "StructTypes.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct GASP_API FGaitSettings
{
	GENERATED_BODY()

	float GetSpeed(const EGait Gait, const FVector& Velocity, const FRotator& ActorRotation,
	               const bool bIsCrouched = false) const
	{
		const FVector& SpeedRange{bIsCrouched ? CrouchSpeed : GetSpeedRangeForGait(Gait)};
		return UE_REAL_TO_FLOAT(InterpolateSpeedForDirection(SpeedRange, Velocity, ActorRotation));
	}

	UCurveVector* GetMovementCurve() const
	{
		return MovementCurve.Get();
	}

	FVector GetSpeedRangeForGait(EGait Gait) const
	{
		static const FVector GaitSpeeds[]{WalkSpeed, RunSpeed, SprintSpeed};
		return Gait >= EGait::Walk && Gait <= EGait::Sprint ? GaitSpeeds[static_cast<int32>(Gait)] : RunSpeed;
	}

	float InterpolateSpeedForDirection(const FVector& SpeedRange, const FVector& Velocity,
	                                   const FRotator& ActorRotation) const
	{
		const float Dir{FGASPMath::CalculateDirection(Velocity, ActorRotation)};
		const float StrafeSpeedMap{StrafeCurve.IsValid() ? StrafeCurve->GetFloatValue(FMath::Abs(Dir)) : 0.f};

		if (StrafeSpeedMap < 1.f)
		{
			return FMath::Lerp(SpeedRange.X, SpeedRange.Y, StrafeSpeedMap);
		}

		return FMath::Lerp(SpeedRange.Y, SpeedRange.Z, StrafeSpeedMap - 1.f);
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (Description = "X = Forward Speed, Y = Strafe Speed, Z = Backwards Speed"))
	FVector WalkSpeed{200.f, 180.f, 150.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (Description = "X = Forward Speed, Y = Strafe Speed, Z = Backwards Speed"))
	FVector RunSpeed{450.f, 400.f, 350.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (Description = "X = Forward Speed, Y = Strafe Speed, Z = Backwards Speed"))
	FVector SprintSpeed{700.f, 0.f, 0.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (Description = "X = Forward Speed, Y = Strafe Speed, Z = Backwards Speed"))
	FVector CrouchSpeed{225.f, 200.f, 180.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<UCurveFloat> StrafeCurve{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<UCurveVector> MovementCurve{};
};

/**
 *
 */
USTRUCT(BlueprintType)
struct GASP_API FCharacterInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxTurnAngle{50.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (ClampMin = 0, ForceUnits = "s"))
	float Speed{0.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AngleVelocity{0.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FlailRate{0.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Velocity{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Acceleration{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector VelocityAcceleration{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform RootTransform{FTransform::Identity};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform ActorTransform{FTransform::Identity};
};

/**
 *
 */
USTRUCT(BlueprintType)
struct GASP_API FMotionMatchingInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<const class UPoseSearchDatabase> PoseSearchDatabase{};
	UPROPERTY(BlueprintReadOnly)
	FVector FutureVelocity{FVector::ZeroVector};
	UPROPERTY(BlueprintReadOnly)
	FVector CurrentVelocity{FVector::ZeroVector};
	UPROPERTY(BlueprintReadOnly)
	FVector PreviousVelocity{FVector::ZeroVector};
	UPROPERTY(BlueprintReadOnly)
	FVector LastNonZeroVector{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> DatabaseTags{};
	UPROPERTY(BlueprintReadOnly, meta = (ClampMin = 0))
	float OrientationAlpha{.2f};
	UPROPERTY(BlueprintReadOnly)
	float PreviousDesiredYawRotation{0.f};
	UPROPERTY(BlueprintReadOnly, meta = (ClampMin = 0))
	float AnimTime{0.f};
	UPROPERTY(BlueprintReadOnly, meta = (ClampMin = 0))
	float PlayRate{0.f};
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<class UAnimationAsset> AnimAsset;
};

/**
 *
 */
USTRUCT(BlueprintType)
struct GASP_API FAnimUtilityNames
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName MovingTraversalCurveName{TEXT("MovingTraversal")};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EnableMotionWarpingCurveName{TEXT("Enable_OrientationWarping")};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AnimationSlotName{TEXT("DefaultSlot")};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName DisableAOCurveName{TEXT("Disable_AO")};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName StopsTag{TEXT("Stops")};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TurnInPlaceTag{TEXT("TurnInPlace")};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PivotsTag{TEXT("Pivots")};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PoseHistoryTag{TEXT("PoseHistory")};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringLegsSlotName{TEXT("Layering_Legs")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringPelvisSlotName{TEXT("Layering_Pelvis")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringHeadSlotName{TEXT("Layering_Head")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringSpineName{TEXT("Layering_Spine")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringSpineAdditiveName{TEXT("Layering_Spine_Add")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringHeadAdditiveName{TEXT("Layering_Head_Add")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringArmLeftAdditiveName{TEXT("Layering_Arm_L_Add")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringArmRightAdditiveName{TEXT("Layering_Arm_R_Add")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringHandLeftName{TEXT("Layering_Hand_L")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringHandRightName{TEXT("Layering_Hand_R")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringHandLeftIKName{TEXT("Enable_HandIK_L")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringArmLeftName{TEXT("Layering_Arm_L")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringHandRightIKName{TEXT("Enable_HandIK_R")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringArmRightName{TEXT("Layering_Arm_R")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringArmLeftLocalSpaceName{TEXT("Layering_Arm_L_LS")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Names")
	FName LayeringArmRightLocalSpaceName{TEXT("Layering_Arm_R_LS")};
};

USTRUCT(BlueprintType)
struct GASP_API FOverlaySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<UAnimInstance> OverlayInstance;
};

USTRUCT(BlueprintType)
struct GASP_API FLayeringState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float HeadAdditiveBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftAdditiveBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftLocalSpaceBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftMeshSpaceBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightAdditiveBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightLocalSpaceBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightMeshSpaceBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float HandLeftBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float HandRightBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float EnableHandLeftIKBlend{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float EnableHandRightIKBlend{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float SpineAdditiveBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float PelvisBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values", Meta = (ClampMin = 0, ClampMax = 1))
	float LegsBlendAmount{0.0f};
};

USTRUCT(BlueprintType)
struct GASP_API FRagdollingState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Character")
	FVector Velocity{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Character", meta = (ForceUnits = "N"))
	float PullForce{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Character", meta = (ClampMin = 0))
	int32 SpeedLimitFrameTimeRemaining{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Character",
		meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float SpeedLimit{0.0f};
};

USTRUCT(BlueprintType)
struct GASP_API FRagdollingAnimationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	FPoseSnapshot FinalRagdollPose;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP", meta = (ClampMin = 0, ClampMax = 1, ForceUnits = "x"))
	float FlailPlayRate{1.0f};
};


USTRUCT(BlueprintType, meta=(ToolTip = "Struct used in State Machine to drive Blend Stack inputs"))
struct GASP_API FGASPBlendStackInputs
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	TWeakObjectPtr<class UAnimationAsset> AnimationAsset{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	bool bLoop{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP", meta = (ClampMin = 0))
	float StartTime{.0f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP", meta = (ClampMin = 0))
	float BlendTime{.0f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	TWeakObjectPtr<const class UBlendProfile> BlendProfile{};
};

USTRUCT(BlueprintType)
struct GASP_API FGASPChooserOutputs
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP", meta = (ClampMin = 0))
	float StartTime{.0f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP", meta = (ClampMin = 0))
	float BlendTime{.3f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	bool bUseMotionMatching{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP", meta = (ClampMin = 0))
	float MMCostLimit{.0f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	FName BlendProfile{NAME_None};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	TArray<FName> Tags;
};

USTRUCT(BlueprintType)
struct GASP_API FGASPTraversalCheckInputs
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	FVector TraceForwardDirection{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	float TraceForwardDistance{.0f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	FVector TraceOriginOffset{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	FVector TraceEndOffset{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	float TraceRadius{.0f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	float TraceHalfHeight{.0f};
};

USTRUCT(BlueprintType)
struct GASP_API FGASPBlendPoses
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP", meta = (ClampMin = 0))
	float BasePoseN{.0f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP", meta = (ClampMin = 0))
	float BasePoseCLF{.0f};
};

USTRUCT(BlueprintType)
struct GASP_API FTraversalCheckResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	FGameplayTag ActionType{FGameplayTag::EmptyTag};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	uint8 bHasFrontLedge : 1{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector FrontLedgeLocation{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector FrontLedgeNormal{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	uint8 bHasBackLedge : 1{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector BackLedgeLocation{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector BackLedgeNormal{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	uint8 bHasBackFloor : 1{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector BackFloorLocation{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float ObstacleHeight{0.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float ObstacleDepth{0.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float BackLedgeHeight{0.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<UPrimitiveComponent> HitComponent{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<const UAnimMontage> ChosenMontage{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (ClampMin = 0))
	float StartTime{0.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (ClampMin = 0))
	float PlayRate{0.f};

	FORCEINLINE FString ToString() const
	{
		return FString::Printf(
			TEXT(
				"Has Front Ledge: %hhd\nHas Back Ledge: %hhd\nHas Back Floor:%hhd\nObstacle Height: %f\nObstacle Depth: "
				"%f\nBack Ledge Height: %f\nChosen Animation: %s\nAnimation Start Time: %f\nAnimation Play Rate: %f"),
			bHasFrontLedge, bHasBackLedge, bHasBackFloor, ObstacleHeight, ObstacleDepth, BackLedgeHeight,
			IsValid(ChosenMontage) ? *ChosenMontage->GetName() : TEXT("nullptr"), StartTime, PlayRate);
	}
};

USTRUCT(BlueprintType)
struct GASP_API FBlendStackMachine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	bool bLoop{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP", meta = (ClampMin = 0))
	float AssetTimeRemaining{.0f};
};
