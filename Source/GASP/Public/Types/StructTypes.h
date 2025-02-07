// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/EnumTypes.h"
#include "Utils/GASPMath.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "StructTypes.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct GASP_API FGait
{
	GENERATED_BODY()

	FGait() = default;

	explicit FGait(const EGait InitialGait)
	{
		*this = InitialGait;
	}

	const bool& IsWalk() const
	{
		return bWalk;
	}

	const bool& IsRun() const
	{
		return bRun;
	}

	const bool& IsSprint() const
	{
		return bSprint;
	}

	void operator=(const EGait NewGate)
	{
		Gait = NewGate;
		bWalk = Gait == EGait::Walk;
		bRun = Gait == EGait::Run;
		bSprint = Gait == EGait::Sprint;
	}

	operator EGait() const
	{
		return Gait;
	}

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bWalk = false;
	UPROPERTY(BlueprintReadOnly)
	bool bRun = true;
	UPROPERTY(BlueprintReadOnly)
	bool bSprint = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGait Gait{EGait::Run};
};

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Description = "X = Forward Speed, Y = Backwards Speed"))
	FVector WalkSpeed{200.f, 180.f, 150.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Description = "X = Forward Speed, Y = Backwards Speed"))
	FVector RunSpeed{450.f, 400.f, 350.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Description = "X = Forward Speed, Y = Backwards Speed"))
	FVector SprintSpeed{700.f, 0.f, 0.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Description = "X = Forward Speed, Y = Backwards Speed"))
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
struct GASP_API FRotationMode
{
	GENERATED_BODY()

	FRotationMode() = default;

	explicit FRotationMode(const ERotationMode InitialRotationMode)
	{
		*this = InitialRotationMode;
	}

	const bool& IsStrafe() const
	{
		return bStrafe;
	}

	const bool& IsOrientToMovement() const
	{
		return bOrientToMovement;
	}

	const bool& IsAim() const
	{
		return bAim;
	}

	void operator=(const ERotationMode NewRotationMode)
	{
		RotationMode = NewRotationMode;
		bStrafe = RotationMode == ERotationMode::Strafe;
		bOrientToMovement = RotationMode == ERotationMode::OrientToMovement;
		bAim = RotationMode == ERotationMode::Aim;
	}

	operator ERotationMode() const
	{
		return RotationMode;
	}

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bStrafe = true;
	UPROPERTY(BlueprintReadOnly)
	bool bOrientToMovement = false;
	UPROPERTY(BlueprintReadOnly)
	bool bAim = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ERotationMode RotationMode{ERotationMode::Strafe};
};

/**
 *
 */
USTRUCT(BlueprintType)
struct GASP_API FMovementState
{
	GENERATED_BODY()

	FMovementState() = default;

	explicit FMovementState(const EMovementState InitialMovementState)
	{
		*this = InitialMovementState;
	}

	const bool& IsIdle() const
	{
		return bIdle;
	}

	const bool& IsMoving() const
	{
		return bMoving;
	}

	void operator=(const EMovementState NewMovementState)
	{
		MovementState = NewMovementState;
		bIdle = MovementState == EMovementState::Idle;
		bMoving = MovementState == EMovementState::Moving;
	}

	operator EMovementState() const
	{
		return MovementState;
	}

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bIdle = true;
	UPROPERTY(BlueprintReadOnly)
	bool bMoving = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMovementState MovementState{EMovementState::Idle};
};

/**
 *
 */
USTRUCT(BlueprintType)
struct GASP_API FMovementMode
{
	GENERATED_BODY()

	FMovementMode() = default;

	explicit FMovementMode(const ECMovementMode InitialMovementMode)
	{
		*this = InitialMovementMode;
	}

	const bool& IsOnGround() const
	{
		return bOnGround;
	}

	const bool& IsInAir() const
	{
		return bInAir;
	}

	void operator=(const ECMovementMode NewMovementMode)
	{
		MovementMode = NewMovementMode;
		bOnGround = MovementMode == ECMovementMode::OnGround;
		bInAir = MovementMode == ECMovementMode::InAir;
	}

	operator ECMovementMode() const
	{
		return MovementMode;
	}

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bOnGround{true};
	UPROPERTY(BlueprintReadOnly)
	bool bInAir{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECMovementMode MovementMode{ECMovementMode::OnGround};
};

/**
 *
 */
USTRUCT(BlueprintType)
struct GASP_API FStanceMode
{
	GENERATED_BODY()

	FStanceMode() = default;

	explicit FStanceMode(const EStanceMode InitialMovementMode)
	{
		*this = InitialMovementMode;
	}

	const bool& IsStand() const
	{
		return bStand;
	}

	const bool& IsCrouch() const
	{
		return bCrouch;
	}

	void operator=(const EStanceMode NewStanceMode)
	{
		StanceMode = NewStanceMode;
		bStand = StanceMode == EStanceMode::Stand;
		bCrouch = StanceMode == EStanceMode::Crouch;
	}

	operator EStanceMode() const
	{
		return StanceMode;
	}

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bStand{true};
	UPROPERTY(BlueprintReadOnly)
	bool bCrouch{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EStanceMode StanceMode{EStanceMode::Stand};
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Speed{0.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Direction{0.f};
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
	UPROPERTY(BlueprintReadOnly)
	float AnimTime{0.f};
	UPROPERTY(BlueprintReadOnly)
	float PlayRate{0.f};
	UPROPERTY(BlueprintReadOnly)
	float TimeRemaining{0.f};
	UPROPERTY(BlueprintReadOnly)
	bool bLoop{false};
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<class UAnimationAsset> AnimAsset;
};

/**
 *
 */
USTRUCT(BlueprintType)
struct GASP_API FAnimCurves
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
};

USTRUCT(BlueprintType)
struct GASP_API FMovementDirectionThreshold
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FL{-60.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FR{60.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BL{-120.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BR{120.f};
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float HeadAdditiveBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftAdditiveBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftLocalSpaceBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmLeftMeshSpaceBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightAdditiveBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightLocalSpaceBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float ArmRightMeshSpaceBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float HandLeftBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float HandRightBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float EnableHandLeftIKBlend{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float EnableHandRightIKBlend{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float SpineAdditiveBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float PelvisBlendAmount{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float LegsBlendAmount{0.0f};
};

USTRUCT(BlueprintType)
struct GASP_API FLayeringNames
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringLegsSlotName{TEXT("Layering_Legs")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringPelvisSlotName{TEXT("Layering_Pelvis")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringHeadSlotName{TEXT("Layering_Head")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringSpineAdditiveName{TEXT("Layering_Spine_Add")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringHeadAdditiveName{TEXT("Layering_Head_Add")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringArmLeftAdditiveName{TEXT("Layering_Arm_L_Add")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringArmRightAdditiveName{TEXT("Layering_Arm_R_Add")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringHandLeftName{TEXT("Layering_Hand_L")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringHandRightName{TEXT("Layering_Hand_R")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringHandLeftIKName{TEXT("Enable_HandIK_L")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringArmLeftName{TEXT("Layering_Arm_L")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringHandRightIKName{TEXT("Enable_HandIK_R")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringArmRightName{TEXT("Layering_Arm_R")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringArmLeftLocalSpaceName{TEXT("Layering_Arm_L_LS")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layering|Values")
	FName LayeringArmRightLocalSpaceName{TEXT("Layering_Arm_R_LS")};
};

USTRUCT(BlueprintType)
struct GASP_API FRagdollingState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Character")
	FVector Velocity{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Character", Meta = (ForceUnits = "N"))
	float PullForce{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GASP", Meta = (ClampMin = 0))
	int32 SpeedLimitFrameTimeRemaining{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GASP", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float SpeedLimit{0.0f};
};

USTRUCT(BlueprintType)
struct GASP_API FRagdollingAnimationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	FPoseSnapshot FinalRagdollPose;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP", Meta = (ClampMin = 0, ClampMax = 1, ForceUnits = "x"))
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	float StartTime{.0f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	float BlendTime{.0f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	TWeakObjectPtr<const class UBlendProfile> BlendProfile{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	TArray<FName> Tags;
};

USTRUCT(BlueprintType)
struct GASP_API FGASPChooserOutputs
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	float StartTime{.0f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	float BlendTime{.3f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
	bool bUseMotionMatching{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP")
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
