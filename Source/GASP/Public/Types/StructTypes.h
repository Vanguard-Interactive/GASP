// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/EnumTypes.h"
#include "Utils/AnimationUtils.h"
#include "Curves/CurveFloat.h"
#include "StructTypes.generated.h"

/**
 * For work with gait in Character or CharacterMovementComponent or other
 */
USTRUCT(BlueprintType)
struct FGait
{
	GENERATED_BODY()

	FGait() = default;

	explicit FGait(const EGait InitialGait)
	{
		*this = InitialGait;
	}

	const bool& isWalk() const { return bWalk; }
	const bool& isRun() const { return bRun; }
	const bool& isSprint() const { return bSprint; }

	void operator=(const EGait NewGate)
	{
		Gait = NewGate;
		bWalk = Gait == EGait::Walk;
		bRun = Gait == EGait::Run;
		bSprint = Gait == EGait::Sprint;
	}

	operator EGait() const { return Gait; }
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bWalk = false;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bRun = true;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bSprint = false;
	UPROPERTY(BlueprintReadOnly)
	EGait Gait{ EGait::Run };
};

/**
 * For work with speed character
 */
USTRUCT(BlueprintType)
struct FGaitSettings
{
	GENERATED_BODY()

	FVector GetSpeedForGait(EGait Gait) const
	{
		static const FVector GaitSpeeds[] = { WalkSpeed, RunSpeed, SprintSpeed };
		return Gait >= EGait::Walk && Gait <= EGait::Sprint ? GaitSpeeds[static_cast<int32>(Gait)] : RunSpeed;
	}

	float GetMappedSpeed(const EGait NewGait, const FVector& Velocity, const FRotator& ActorRotation) const
	{
		const float Dir = UAnimationUtils::CalculateDirection(Velocity, ActorRotation);
		const float StrafeSpeedMap = StrafeCurve.IsValid() ? StrafeCurve->GetFloatValue(FMath::Abs(Dir)): 0.f;
		const FVector Speed = GetSpeedForGait(NewGait);
		FVector2f OutRange{}, InRange{};
		if (StrafeSpeedMap < 1.f)
		{
			OutRange = FVector2f(Speed.X, Speed.Y);
			InRange = FVector2f(0.f, 1.f);
		}
		else
		{
			OutRange = FVector2f(Speed.Y, Speed.Z);
			InRange = FVector2f(1.f, 2.f);
		} 
		return FMath::GetMappedRangeValueClamped(InRange, OutRange, StrafeSpeedMap);
	}

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Description = "X = Forward Speed, Y = Strafe Speed, Z = Backwards Speed"))
	FVector WalkSpeed{200.f, 175.f, 150.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Description = "X = Forward Speed, Y = Strafe Speed, Z = Backwards Speed"))
	FVector RunSpeed{450.f, 400.f, 350.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly , meta = (Description = "X = Forward Speed, Y = Strafe Speed, Z = Backwards Speed"))
	FVector SprintSpeed{ 700.f, 700.f, 700.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<UCurveFloat> StrafeCurve{nullptr};
};

USTRUCT(BlueprintType)
struct FRotationMode
{
	GENERATED_BODY()

	FRotationMode() = default;

	explicit FRotationMode(const ERotationMode InitialRotationMode)
	{
		*this = InitialRotationMode;
	}

	const bool& isStrafe() const { return bStrafe; }
	const bool& isOrientToMovement() const { return bOrientToMovement; }

	void operator=(const ERotationMode NewRotationMode)
	{
		RotationMode = NewRotationMode;
		bStrafe = RotationMode == ERotationMode::Strafe;
		bOrientToMovement = RotationMode == ERotationMode::OrientToMovement;
	}

	operator ERotationMode() const { return RotationMode; }
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bStrafe = true;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bOrientToMovement = false;
	UPROPERTY(BlueprintReadOnly)
	ERotationMode RotationMode{ ERotationMode::Strafe };
};

USTRUCT(BlueprintType)
struct FMovementState
{
	GENERATED_BODY()

	FMovementState() = default;

	explicit FMovementState(const EMovementState InitialMovementState)
	{
		*this = InitialMovementState;
	}

	const bool& isIdle() const { return bIdle; }
	const bool& isMoving() const { return bMoving; }

	void operator=(const EMovementState NewMovementState)
	{
		MovementState = NewMovementState;
		bIdle = MovementState == EMovementState::Idle;
		bMoving = MovementState == EMovementState::Moving;
	}

	operator EMovementState() const { return MovementState; }
protected:
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bIdle = true;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bMoving = false;
	UPROPERTY(BlueprintReadOnly)
	EMovementState MovementState{ EMovementState::Idle };
};

USTRUCT(BlueprintType)
struct FMovementMode
{
	GENERATED_BODY()

	FMovementMode() = default;

	explicit FMovementMode(const ECMovementMode InitialMovementMode)
	{
		*this = InitialMovementMode;
	}

	const bool& isOnGround() const { return bOnGround; }
	const bool& isInAir() const { return bInAir; }

	void operator=(const ECMovementMode NewMovementMode)
	{
		MovementMode = NewMovementMode;
		bOnGround = MovementMode == ECMovementMode::OnGround;
		bInAir = MovementMode == ECMovementMode::InAir;
	}

	operator ECMovementMode() const { return MovementMode; }
	
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bOnGround{ true };
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bInAir{ false };
	UPROPERTY(BlueprintReadOnly)
	ECMovementMode MovementMode{ ECMovementMode::OnGround };
};

USTRUCT(BlueprintType)
struct FStanceMode
{
	GENERATED_BODY()

	FStanceMode() = default;

	explicit FStanceMode(const EStanceMode InitialMovementMode)
	{
		*this = InitialMovementMode;
	}

	const bool& isStand() const { return bStand; }
	const bool& isCrouch() const { return bCrouch; }

	void operator=(const EStanceMode NewStanceMode)
	{
		StanceMode = NewStanceMode;
		bStand = StanceMode == EStanceMode::Stand;
		bCrouch = StanceMode == EStanceMode::Crouch;
	}

	operator EStanceMode() const { return StanceMode; }
	
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bStand{ true };
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bCrouch{ false };
	UPROPERTY(BlueprintReadOnly)
	EStanceMode StanceMode{ EStanceMode::Stand };
};

USTRUCT(BlueprintType)
struct FCharacterInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxTurnAngle{ 50.f };
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float Speed{ 0.f };
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FVector Velocity{ FVector::ZeroVector };
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float Direction{ 0.f };
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FVector Acceleration{ FVector::ZeroVector };
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	bool HasVelocity{ false };
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FTransform RootTransform{ };
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FTransform ActorTransform{ };
};

USTRUCT(BlueprintType)
struct FMotionMatchingInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<const class UPoseSearchDatabase> PoseSearchDatabase{ };
	UPROPERTY(BlueprintReadOnly)
	FVector FutureVelocity{ FVector::ZeroVector };
	UPROPERTY(BlueprintReadOnly)
	FVector CurrentVelocity{ FVector::ZeroVector };
	UPROPERTY(BlueprintReadOnly)
	FVector PreviousVelocity{ FVector::ZeroVector };
	UPROPERTY(BlueprintReadOnly)
	FVector LastNonZeroVector{ FVector::ZeroVector };
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> DatabaseTags{ };
	UPROPERTY(BlueprintReadOnly, meta = (ClampMin=0))
	float OrientationAlpha{ .2f };
	UPROPERTY(BlueprintReadOnly)
	float PreviousDesiredYawRotation{ 0.f };
	UPROPERTY(BlueprintReadOnly)
	float AnimTime{ 0.f };
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<class UAnimationAsset> AnimAsset;
};


USTRUCT(BlueprintType)
struct FAnimCurves
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName MovingTraversalCurveName{ TEXT("MovingTraversal") };
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EnableMotionWarpingCurveName{ TEXT("Enable_OrientationWarping") };
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AnimationSlotName{ TEXT("DefaultSlot") };
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName DisableAOCurveName{ TEXT("Disable_AO") };
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName StopsTag{ TEXT("Stops") };
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TurnInPlaceTag{ TEXT("TurnInPlace") };
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PivotsTag{ TEXT("Pivots") };
};