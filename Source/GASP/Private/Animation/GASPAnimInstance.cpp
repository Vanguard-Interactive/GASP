// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/GASPAnimInstance.h"
#include "Actors/GASPCharacter.h"
#include "Components/GASPCharacterMovementComponent.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "Utils/GASPMath.h"
#include "Utils/OverlayLayeringDataAsset.h"
#include "PoseSearch/MotionMatchingAnimNodeLibrary.h"
#include "ChooserFunctionLibrary.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "AnimationWarpingLibrary.h"
#include "BlendStack/BlendStackAnimNodeLibrary.h"
#include "BoneControllers/AnimNode_FootPlacement.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASPAnimInstance)

struct FAnimNode_Root;

void UGASPAnimInstance::OnLanded(const FHitResult& HitResult)
{
	bLanded = true;

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]() { bLanded = false; });
}

void UGASPAnimInstance::OnOverlayStateChanged(const EOverlayState NewOverlayState)
{
	if (!CachedCharacter.IsValid())
	{
		return;
	}

	OverlayState = NewOverlayState;

	USkeletalMeshComponent* Mesh = CachedCharacter->GetMesh();
	if (!IsValid(Mesh))
	{
		return;
	}

	const UOverlayLayeringDataAsset* DataAsset{
		static_cast<UOverlayLayeringDataAsset*>(
			UChooserFunctionLibrary::EvaluateChooser(this, OverlayTable, UOverlayLayeringDataAsset::StaticClass()))
	};
	if (!IsValid(DataAsset))
	{
		return;
	}

	Mesh->LinkAnimClassLayers(DataAsset->GetOverlayAnimInstance());
}

EPoseSearchInterruptMode UGASPAnimInstance::GetMatchingInterruptMode() const
{
	return MovementMode != PreviousMovementMode || (MovementMode.IsOnGround() && (MovementState != PreviousMovementState
		       || (Gait != PreviousGait && MovementState.IsMoving()) || StanceMode != PreviousStanceMode))
		       ? EPoseSearchInterruptMode::InterruptOnDatabaseChange
		       : EPoseSearchInterruptMode::DoNotInterrupt;
}

EOffsetRootBoneMode UGASPAnimInstance::GetOffsetRootRotationMode() const
{
	return IsSlotActive(AnimNames.AnimationSlotName) ? EOffsetRootBoneMode::Release : EOffsetRootBoneMode::Accumulate;
}

EOffsetRootBoneMode UGASPAnimInstance::GetOffsetRootTranslationMode() const
{
	if (IsSlotActive(AnimNames.AnimationSlotName))
	{
		return EOffsetRootBoneMode::Release;
	}
	if (MovementMode.IsOnGround())
	{
		return MovementState.IsMoving() ? EOffsetRootBoneMode::Interpolate : EOffsetRootBoneMode::Release;
	}
	if (MovementMode.IsInAir())
	{
		return EOffsetRootBoneMode::Release;
	}
	return EOffsetRootBoneMode::Release;
}

float UGASPAnimInstance::GetOffsetRootTranslationHalfLife() const
{
	return MovementState.IsMoving() ? .3f : .1f;
}

EOrientationWarpingSpace UGASPAnimInstance::GetOrientationWarpingSpace() const
{
	return OffsetRootBoneEnabled
		       ? EOrientationWarpingSpace::RootBoneTransform
		       : EOrientationWarpingSpace::ComponentTransform;
}

float UGASPAnimInstance::GetAOYaw() const
{
	return RotationMode.IsOrientToMovement() ? 0.f : GetAOValue().X;
}

bool UGASPAnimInstance::IsEnableSteering() const
{
	return MovementState.IsMoving() || MovementMode.IsInAir();
}

void UGASPAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	CachedCharacter = Cast<AGASPCharacter>(TryGetPawnOwner());
	if (!CachedCharacter.IsValid())
	{
		return;
	}

	CachedMovement = CachedCharacter->FindComponentByClass<UGASPCharacterMovementComponent>();
	if (!CachedMovement.IsValid())
	{
		return;
	}
	CachedCharacter->OverlayStateChanged.AddUniqueDynamic(this, &ThisClass::OnOverlayStateChanged);
}

void UGASPAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CachedCharacter = Cast<AGASPCharacter>(TryGetPawnOwner());
	if (!CachedCharacter.IsValid())
	{
		return;
	}

	CachedMovement = CachedCharacter->FindComponentByClass<UGASPCharacterMovementComponent>();
	if (!CachedMovement.IsValid())
	{
		return;
	}

	CachedCharacter->LandedDelegate.AddUniqueDynamic(this, &ThisClass::OnLanded);
}

void UGASPAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::NativeThreadSafeUpdateAnimation"),
	                            STAT_UGASPAnimInstance_NativeThreadSafeUpdateAnimation, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!CachedCharacter.IsValid() || !CachedMovement.IsValid())
	{
		return;
	}

	Gait = CachedCharacter->GetGait();
	StanceMode = CachedCharacter->GetStanceMode();
	RotationMode = CachedCharacter->GetRotationMode();
	MovementState = CachedCharacter->GetMovementState();
	MovementMode = CachedCharacter->GetMovementMode();
	LocomotionAction = CachedCharacter->GetLocomotionAction();

	RefreshCVar();
	RefreshEssentialValues(DeltaSeconds);
	RefreshTrajectory(DeltaSeconds);
	RefreshMovementDirection(DeltaSeconds);
	RefreshOverlaySettings(DeltaSeconds);
	RefreshLayering();
	RefreshTargetRotation();

	if (LocomotionAction == ELocomotionAction::Ragdoll)
	{
		RefreshRagdollValues(DeltaSeconds);
	}
}

void UGASPAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::NativeUpdateAnimation"),
	                            STAT_UGASPAnimInstance_NativeUpdateAnimation, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedCharacter.IsValid() || !CachedMovement.IsValid())
	{
		return;
	}
}

void UGASPAnimInstance::PreUpdateAnimation(float DeltaSeconds)
{
	Super::PreUpdateAnimation(DeltaSeconds);

	PreviousCharacterInfo = CharacterInfo;
	PreviousGait = Gait;
	PreviousMovementMode = MovementMode;
	PreviousRotationMode = RotationMode;
	PreviousMovementState = MovementState;
	PreviousStanceMode = StanceMode;
	PreviousLocomotionAction = LocomotionAction;
}

void UGASPAnimInstance::RefreshCVar()
{
	// Get console variables
	const auto& OffsetRootEnabled = IConsoleManager::Get().FindConsoleVariable(
		TEXT("a.animnode.offsetrootbone.enable"));
	const auto& MMLod = IConsoleManager::Get().FindConsoleVariable(TEXT("DDCvar.MMDatabaseLOD"));

	if (OffsetRootEnabled)
	{
		OffsetRootBoneEnabled = OffsetRootEnabled->GetBool();
	}
	if (MMLod)
	{
		MMDatabaseLOD = MMLod->GetInt();
	}
}

void UGASPAnimInstance::RefreshTrajectory(const float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::RefreshTrajectory"),
	                            STAT_UGASPAnimInstance_RefreshTrajectory, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	const auto& TrajectoryData{
		CharacterInfo.Speed > 0.f
			? TrajectoryGenerationData_Moving
			: TrajectoryGenerationData_Idle
	};
	FPoseSearchQueryTrajectory OutTrajectory{};

	UPoseSearchTrajectoryLibrary::PoseSearchGenerateTrajectory(this, TrajectoryData, DeltaSeconds, Trajectory,
	                                                           MotionMatching.PreviousDesiredYawRotation, OutTrajectory,
	                                                           -1.f, 30, .1f, 15);

	const TArray<AActor*> IgnoredActors{};
	UPoseSearchTrajectoryLibrary::HandleTrajectoryWorldCollisions(
		CachedCharacter.Get(), this, OutTrajectory, true, .01f,
		Trajectory, CollisionResults, TraceTypeQuery_MAX, false,
		IgnoredActors, EDrawDebugTrace::None, true, 150.f);

	UPoseSearchTrajectoryLibrary::GetTrajectoryVelocity(Trajectory, -.3f, -.2f, MotionMatching.PreviousVelocity, false);
	UPoseSearchTrajectoryLibrary::GetTrajectoryVelocity(Trajectory, .0f, .2f, MotionMatching.CurrentVelocity, false);
	UPoseSearchTrajectoryLibrary::GetTrajectoryVelocity(Trajectory, .4f, .5f, MotionMatching.FutureVelocity, false);
}

void UGASPAnimInstance::RefreshMovementDirection(float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::MovementDirection"),
	                            STAT_UGASPAnimInstance_MovementDirection, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);
	PreviousMovementDirection = MovementDirection;
	if (MovementState.IsIdle())
	{
		return;
	}

	CharacterInfo.Direction = FGASPMath::CalculateDirection(CharacterInfo.Velocity.GetSafeNormal(),
	                                                        CharacterInfo.ActorTransform.Rotator());

	if (RotationMode.IsOrientToMovement() || Gait.IsSprint())
	{
		MovementDirection = EMovementDirection::F;
		return;
	}

	MovementDirectionThreshold = GetMovementDirectionThresholds();

	if (CharacterInfo.Direction >= MovementDirectionThreshold.FL && CharacterInfo.Direction <=
		MovementDirectionThreshold.FR)
	{
		MovementDirection = EMovementDirection::F;
		return;
	}

	if (CharacterInfo.Direction >= MovementDirectionThreshold.BL && CharacterInfo.Direction <=
		MovementDirectionThreshold.FL)
	{
		MovementDirection = MovementDirectionBias == EMovementDirectionBias::LeftFootForward
			                    ? EMovementDirection::LL
			                    : EMovementDirection::LR;
		return;
	}

	if (CharacterInfo.Direction >= MovementDirectionThreshold.BR && CharacterInfo.Direction <=
		MovementDirectionThreshold.FR)
	{
		MovementDirection = MovementDirectionBias == EMovementDirectionBias::RightFootForward
			                    ? EMovementDirection::RR
			                    : EMovementDirection::RL;
		return;
	}

	MovementDirection = EMovementDirection::B;
}

float UGASPAnimInstance::GetMatchingBlendTime() const
{
	if (MovementMode.IsInAir())
	{
		return GetLandVelocity() > 100.f ? .15f : .5f;
	}

	return PreviousMovementMode.IsOnGround() ? .5f : .2f;
}

FFootPlacementPlantSettings UGASPAnimInstance::GetPlantSettings() const
{
	return BlendStackInputs.Tags.Contains(AnimNames.StopsTag)
		       ? PlantSettings_Stops
		       : PlantSettings_Default;
	// return MotionMatching.DatabaseTags.Contains(AnimNames.StopsTag) ? PlantSettings_Stops : PlantSettings_Default;
}

FFootPlacementInterpolationSettings UGASPAnimInstance::GetPlantInterpolationSettings() const
{
	return BlendStackInputs.Tags.Contains(AnimNames.StopsTag)
		       ? InterpolationSettings_Stops
		       : InterpolationSettings_Default;
	// return MotionMatching.DatabaseTags.Contains(AnimNames.StopsTag)
	// 	       ? InterpolationSettings_Stops
	// 	       : InterpolationSettings_Default;
}

float UGASPAnimInstance::GetMatchingNotifyRecencyTimeOut() const
{
	static constexpr float RecencyTimeOuts[]{.2f, .2f, .16f};

	return Gait >= EGait::Walk && Gait <= EGait::Sprint ? RecencyTimeOuts[static_cast<int32>(EGait(Gait))] : .2f;
}

FMovementDirectionThreshold UGASPAnimInstance::GetMovementDirectionThresholds() const
{
	switch (MovementDirection)
	{
	case EMovementDirection::F:
	case EMovementDirection::B:
		return FMovementDirectionThreshold(-60.f, 60.f, -120.f, 120.f);
	case EMovementDirection::LL:
	case EMovementDirection::LR:
	case EMovementDirection::RL:
	case EMovementDirection::RR:
		if (IsPivoting())
		{
			return FMovementDirectionThreshold(-60.f, 60.f, -120.f, 120.f);
		}
		return BlendStackInputs.bLoop && !RotationMode.IsAim()
			       ? FMovementDirectionThreshold(-60.f, 60.f, -140.f, 140.f)
			       : FMovementDirectionThreshold(-40.f, 40.f, -140.f, 140.f);
	default:
		return FMovementDirectionThreshold(-60.f, 60.f, -120.f, 120.f);
	}
}

FPoseSnapshot& UGASPAnimInstance::SnapshotFinalRagdollPose()
{
	check(IsInGameThread())

	// Save a snapshot of the current ragdoll pose for use in animation graph to blend out of the ragdoll.
	SnapshotPose(RagdollingState.FinalRagdollPose);

	return RagdollingState.FinalRagdollPose;
}

bool UGASPAnimInstance::IsStarting() const
{
	return MotionMatching.FutureVelocity.Size2D() > CharacterInfo.Velocity.Size2D() + 100.f && !MotionMatching.
		DatabaseTags.Contains(AnimNames.PivotsTag) && MovementState.IsMoving() && CachedMovement.IsValid();
}

bool UGASPAnimInstance::IsPivoting() const
{
	if (MotionMatching.FutureVelocity.IsNearlyZero() || CharacterInfo.Velocity.IsNearlyZero())
	{
		return false;
	}

	// const float DotProduct{
	// 	UE_REAL_TO_FLOAT(FVector::DotProduct(MotionMatching.FutureVelocity.GetSafeNormal(),
	// 		CharacterInfo.Velocity.GetSafeNormal()))
	// };
	// const float Threshold{FMath::Cos(FMath::DegreesToRadians(RotationMode.IsOrientToMovement() ? 45.f : 4.f))};
	//
	// return DotProduct <= Threshold && MovementState.IsMoving();

	auto InRange = [this](float Speed)
	{
		static const TMap<EGait, FVector2D> GaitRanges = {
			{EGait::Walk, {50.f, 200.f}},
			{EGait::Run, {200.f, 550.f}},
			{EGait::Sprint, {200.f, 700.f}},
		};

		const FVector2D* MinMax = GaitRanges.Find(Gait);
		return Speed > MinMax->X && Speed < MinMax->Y;
	};

	auto ClampedSpeed = [this](float Speed)
	{
		if (StanceMode.IsCrouch())
		{
			return 65.f;
		}

		static const TMap<EGait, FVector4f> InOutRanges = {
			{EGait::Walk, FVector4f(150.f, 200.f, 70.f, 60.f)},
			{EGait::Run, FVector4f(300.f, 500.f, 70.f, 60.f)},
			{EGait::Sprint, FVector4f(300.f, 700.f, 60.f, 50.f)}
		};
		const FVector4f* InOut = InOutRanges.Find(Gait);
		return FMath::GetMappedRangeValueClamped<float, float>({InOut->X, InOut->Y},
		                                                       {InOut->Z, InOut->W}, Speed);
	};

	return InRange(CharacterInfo.Speed) && FMath::Abs(GetTrajectoryTurnAngle()) >= ClampedSpeed(CharacterInfo.Speed) &&
		MovementState.IsMoving();
}

bool UGASPAnimInstance::ShouldTurnInPlace() const
{
	const FVector ActorForward{CharacterInfo.ActorTransform.GetRotation().GetForwardVector()};
	const FVector RootForward{CharacterInfo.RootTransform.GetRotation().GetForwardVector()};

	if (ActorForward.IsNearlyZero() || RootForward.IsNearlyZero())
	{
		return false;
	}

	const float DotProduct{
		UE_REAL_TO_FLOAT(FVector::DotProduct(ActorForward.GetSafeNormal(), RootForward.GetSafeNormal()))
	};
	const float Threshold{FMath::Cos(FMath::DegreesToRadians(CharacterInfo.MaxTurnAngle))};

	return DotProduct <= Threshold && RotationMode.IsAim() && MovementState.IsIdle();
}

bool UGASPAnimInstance::ShouldSpin() const
{
	const FVector ActorForward{CharacterInfo.ActorTransform.GetRotation().GetForwardVector()};
	const FVector RootForward{CharacterInfo.RootTransform.GetRotation().GetForwardVector()};

	if (ActorForward.IsNearlyZero() || RootForward.IsNearlyZero())
	{
		return false;
	}

	const float DotProduct{
		UE_REAL_TO_FLOAT(FVector::DotProduct(ActorForward.GetSafeNormal(), RootForward.GetSafeNormal()))
	};
	const float Threshold{FMath::Cos(FMath::DegreesToRadians(130.f))};

	return DotProduct <= Threshold && CharacterInfo.Speed >= 150.f && !MotionMatching.DatabaseTags.Contains(
		AnimNames.PivotsTag);
}

bool UGASPAnimInstance::JustLanded_Light() const
{
	return FMath::Abs(CharacterInfo.Velocity.Z) < FMath::Abs(HeavyLandSpeedThreshold) && bLanded;
}

bool UGASPAnimInstance::JustLanded_Heavy() const
{
	return FMath::Abs(CharacterInfo.Velocity.Z) >= FMath::Abs(HeavyLandSpeedThreshold) && bLanded;
}

bool UGASPAnimInstance::JustTraversed() const
{
	const float DotProduct{
		UE_REAL_TO_FLOAT(FVector::DotProduct(MotionMatching.FutureVelocity.GetSafeNormal(),
			CharacterInfo.Velocity.GetSafeNormal()))
	};
	const float Threshold{FMath::Cos(FMath::DegreesToRadians(CharacterInfo.MaxTurnAngle))};

	return !IsSlotActive(AnimNames.AnimationSlotName) && GetCurveValue(AnimNames.MovingTraversalCurveName) > 0.f &&
		DotProduct >= Threshold;
}

float UGASPAnimInstance::GetLandVelocity() const
{
	return CharacterInfo.Velocity.Z;
}

bool UGASPAnimInstance::PlayLand() const
{
	return MovementMode.IsOnGround() && PreviousMovementMode.IsInAir();
}

bool UGASPAnimInstance::PlayMovingLand() const
{
	return MovementMode.IsOnGround() && PreviousMovementMode.IsInAir() && FMath::Abs(GetTrajectoryTurnAngle()) <=
		120.f;
}

float UGASPAnimInstance::GetTrajectoryTurnAngle() const
{
	return FMath::RadiansToDegrees(
		FMath::Acos(FVector::DotProduct(MotionMatching.FutureVelocity, CharacterInfo.Velocity)));
}

FVector2D UGASPAnimInstance::GetLeanAmount() const
{
	if (!CachedCharacter.IsValid())
	{
		return FVector2D::ZeroVector;
	}

	const FVector RelAccel{GetRelativeAcceleration()};
	return FVector2D(
		RelAccel * FMath::GetMappedRangeValueClamped<float, float>({200.f, 500.f}, {.5f, 1.f},
		                                                           CharacterInfo.Speed));
}

FVector UGASPAnimInstance::GetRelativeAcceleration() const
{
	if (!CachedMovement.IsValid())
	{
		return FVector::ZeroVector;
	}

	const float Dot{UE_REAL_TO_FLOAT(FVector::DotProduct(CharacterInfo.Velocity, CharacterInfo.Acceleration))};
	const float MaxAccelValue{
		Dot > 0.f ? CachedMovement->GetMaxAcceleration() : CachedMovement->GetMaxBrakingDeceleration()
	};

	const FVector ClampedAcceleration = CharacterInfo.VelocityAcceleration.GetClampedToMaxSize(MaxAccelValue) /
		MaxAccelValue;
	return CharacterInfo.ActorTransform.GetRotation().UnrotateVector(ClampedAcceleration);
}

void UGASPAnimInstance::RefreshMotionMatchingMovement(const FAnimUpdateContext& Context,
                                                      const FAnimNodeReference& Node)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::MotionMatching"),
	                            STAT_UGASPAnimInstance_MotionMatching, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);
	if (!LocomotionTable)
	{
		return;
	}

	EAnimNodeReferenceConversionResult Result{};
	const FMotionMatchingAnimNodeReference Reference{
		UMotionMatchingAnimNodeLibrary::ConvertToMotionMatchingNode(Node, Result)
	};
	if (Result == EAnimNodeReferenceConversionResult::Failed)
	{
		return;
	}

	const TArray<UObject*> Objects{
		UChooserFunctionLibrary::EvaluateChooserMulti(this, LocomotionTable, UPoseSearchDatabase::StaticClass())
	};
	TArray<UPoseSearchDatabase*> Databases{};

	Algo::Transform(Objects, Databases, [](UObject* Object) { return static_cast<UPoseSearchDatabase*>(Object); });
	if (Databases.IsEmpty())
	{
		return;
	}

	UMotionMatchingAnimNodeLibrary::SetDatabasesToSearch(Reference, Databases, GetMatchingInterruptMode());
}

void UGASPAnimInstance::RefreshMatchingPostSelection(const FAnimUpdateContext& Context,
                                                     const FAnimNodeReference& Node)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::RefreshMotionMatchingPostSelection"),
	                            STAT_UGASPAnimInstance_RefreshMotionMatchingPostSelection, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);
	EAnimNodeReferenceConversionResult Result{};
	const FMotionMatchingAnimNodeReference Reference{
		UMotionMatchingAnimNodeLibrary::ConvertToMotionMatchingNode(Node, Result)
	};
	if (Result == EAnimNodeReferenceConversionResult::Failed)
	{
		return;
	}

	FPoseSearchBlueprintResult OutResult{};
	bool bIsValidResult{};

	UMotionMatchingAnimNodeLibrary::GetMotionMatchingSearchResult(Reference, OutResult, bIsValidResult);
	MotionMatching.PoseSearchDatabase = OutResult.SelectedDatabase;
}

void UGASPAnimInstance::RefreshOffsetRoot(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!OffsetRootBoneEnabled)
	{
		return;
	}

	const FTransform TargetTransform{UAnimationWarpingLibrary::GetOffsetRootTransform(Node)};
	FRotator OffsetRotation{TargetTransform.Rotator()};
	OffsetRotation.Yaw += 90.f;

	CharacterInfo.RootTransform = {OffsetRotation, TargetTransform.GetLocation(), FVector::OneVector};
}

FQuat UGASPAnimInstance::GetDesiredFacing() const
{
	// return Trajectory.GetSampleAtTime(.5f, false).Facing;

	const FQuat DesiredFacing = FQuat(TargetRotation);
	const FQuat Offset = FQuat({0.f, -90.f, 0.f});
	return DesiredFacing * Offset;
}

void UGASPAnimInstance::RefreshBlendStack(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::RefreshBlendStack"),
	                            STAT_UGASPAnimInstance_RefreshBlendStack, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);
	MotionMatching.AnimTime = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAssetTime(Node);
	MotionMatching.AnimAsset = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAsset(Node);
	MotionMatching.PlayRate = GetDynamicPlayRate(Node);
	EAnimNodeReferenceConversionResult Result{};
	const FBlendStackAnimNodeReference Reference{
		UBlendStackAnimNodeLibrary::ConvertToBlendStackNode(Node, Result)
	};
	if (Result == EAnimNodeReferenceConversionResult::Succeeded)
	{
		MotionMatching.bLoop = UBlendStackAnimNodeLibrary::IsCurrentAssetLooping(Reference);
		MotionMatching.TimeRemaining = UBlendStackAnimNodeLibrary::GetCurrentAssetTimeRemaining(Reference);
	}

	const UAnimSequence* NewAnimSequence{Cast<UAnimSequence>(MotionMatching.AnimAsset)};
	if (!NewAnimSequence)
	{
		return;
	}

	UAnimationWarpingLibrary::GetCurveValueFromAnimation(NewAnimSequence, AnimNames.EnableMotionWarpingCurveName,
	                                                     MotionMatching.AnimTime, MotionMatching.OrientationAlpha);
}

void UGASPAnimInstance::RefreshEssentialValues(const float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::RefreshEssentialValues"),
	                            STAT_UGASPAnimInstance_RefreshEssentialValues, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);
	CharacterInfo.ActorTransform = CachedCharacter->GetActorTransform();

	if (!OffsetRootBoneEnabled)
	{
		CharacterInfo.RootTransform = CharacterInfo.ActorTransform;
	}

	CharacterInfo.Acceleration = CachedCharacter->GetReplicatedAcceleration();

	// Refresh velocity variables
	CharacterInfo.Velocity = CachedMovement->Velocity;
	CharacterInfo.Speed = CharacterInfo.Velocity.Size2D();

	// Calculate rate of change velocity
	const float SmoothingFactor = CachedCharacter->IsLocallyControlled() ? 1.0f : 0.5f;
	FVector VelocityAcceleration = (CharacterInfo.Velocity - PreviousCharacterInfo.Velocity) / FMath::Max(
		DeltaSeconds, .001f);
	CharacterInfo.VelocityAcceleration = FMath::Lerp(PreviousCharacterInfo.VelocityAcceleration,
	                                                 VelocityAcceleration,
	                                                 SmoothingFactor);

	if (MovementState.IsMoving())
	{
		MotionMatching.LastNonZeroVector = CharacterInfo.Velocity;
	}
}

void UGASPAnimInstance::RefreshRagdollValues(const float DeltaSeconds)
{
	static constexpr auto ReferenceSpeed{1000.0f};
	RagdollingState.FlailPlayRate = FMath::Lerp(
		0.f, 1.f, CachedCharacter->GetRagdollingState().Velocity.Size() / ReferenceSpeed);
}

bool UGASPAnimInstance::IsEnabledAO() const
{
	return FMath::Abs(GetAOValue().X) <= 115.f && !RotationMode.IsOrientToMovement() && GetSlotMontageLocalWeight(
		AnimNames.AnimationSlotName) < .5f;
}

FVector2D UGASPAnimInstance::GetAOValue() const
{
	if (!CachedCharacter.IsValid())
	{
		return FVector2D::ZeroVector;
	}

	FRotator DeltaRot{
		((CachedCharacter->IsLocallyControlled()
			  ? CachedCharacter->GetControlRotation()
			  : CachedCharacter->GetBaseAimRotation())
			- CharacterInfo.RootTransform.Rotator())
		.GetNormalized()
	};

	return FMath::Lerp({DeltaRot.Yaw, DeltaRot.Pitch}, FVector2D::ZeroVector,
	                   GetCurveValue(AnimNames.DisableAOCurveName));
}

void UGASPAnimInstance::RefreshOverlaySettings(float DeltaTime)
{
	const float ClampedYawAxis = FMath::ClampAngle(GetAOValue().X, -90.f, 90.f) / 6.f;
	SpineRotation.Yaw = FMath::FInterpTo(SpineRotation.Yaw, ClampedYawAxis, DeltaTime, 60.f);
}

void UGASPAnimInstance::RefreshLayering()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::RefreshLayering"),
	                            STAT_UGASPAnimInstance_RefreshLayering, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	LayeringState.SpineAdditiveBlendAmount = GetCurveValue(LayeringCurveNames.LayeringSpineAdditiveName);
	LayeringState.HeadAdditiveBlendAmount = GetCurveValue(LayeringCurveNames.LayeringHeadAdditiveName);
	LayeringState.ArmLeftAdditiveBlendAmount = GetCurveValue(LayeringCurveNames.LayeringArmLeftAdditiveName);
	LayeringState.ArmRightAdditiveBlendAmount = GetCurveValue(LayeringCurveNames.LayeringArmRightAdditiveName);

	LayeringState.HandLeftBlendAmount = GetCurveValue(LayeringCurveNames.LayeringHandLeftName);
	LayeringState.HandRightBlendAmount = GetCurveValue(LayeringCurveNames.LayeringHandRightName);

	LayeringState.EnableHandLeftIKBlend = FMath::Lerp(0.f, GetCurveValue(LayeringCurveNames.LayeringHandLeftIKName),
	                                                  GetCurveValue(LayeringCurveNames.LayeringArmLeftName));
	LayeringState.EnableHandRightIKBlend = FMath::Lerp(
		0.f, GetCurveValue(LayeringCurveNames.LayeringHandRightIKName),
		GetCurveValue(LayeringCurveNames.LayeringArmRightName));

	LayeringState.ArmLeftLocalSpaceBlendAmount = GetCurveValue(LayeringCurveNames.LayeringArmLeftLocalSpaceName);
	LayeringState.ArmLeftMeshSpaceBlendAmount = UE_REAL_TO_FLOAT(
		1.f - FMath::FloorToInt(LayeringState.ArmLeftLocalSpaceBlendAmount));

	LayeringState.ArmRightLocalSpaceBlendAmount = GetCurveValue(LayeringCurveNames.LayeringArmRightLocalSpaceName);
	LayeringState.ArmRightMeshSpaceBlendAmount = UE_REAL_TO_FLOAT(
		1.f - FMath::FloorToInt(LayeringState.ArmRightLocalSpaceBlendAmount));
}

void UGASPAnimInstance::SetBlendStackAnimFromChooser(const FAnimNodeReference& Node, EStateMachineState NewState,
                                                     bool bForceBlend)
{
	StateMachineState = NewState;
	PreviousBlendStackInputs = BlendStackInputs;

	bNoValidAnim = false;
	bNotifyTransition_ReTransition = false;
	bNotifyTransition_ToLoop = false;

	// TODO: Test this code
	// Evaluate chooser
	FGASPChooserOutputs ChooserOutputs;
	FChooserEvaluationContext Context;
	Context.AddObjectParam(this);
	Context.AddStructParam(ChooserOutputs);
	const FInstancedStruct InstancedStruct = UChooserFunctionLibrary::MakeEvaluateChooser(StateMachineTable);

	const TArray<UObject*> Objects{
		UChooserFunctionLibrary::EvaluateObjectChooserBaseMulti(Context, InstancedStruct,
		                                                        UAnimationAsset::StaticClass(), false)
	};

	if (Objects.IsEmpty())
	{
		bNoValidAnim = true;
		return;
	}

	// Update blend stack inputs
	BlendStackInputs.AnimationAsset = Cast<UAnimationAsset>(Objects[0]);
	UpdateAnimationLoopingFlag(BlendStackInputs);
	BlendStackInputs.StartTime = ChooserOutputs.StartTime;
	BlendStackInputs.BlendTime = ChooserOutputs.BlendTime;
	BlendStackInputs.BlendProfile = GetBlendProfileByName(ChooserOutputs.BlendProfile);
	BlendStackInputs.Tags = ChooserOutputs.Tags;

	if (ChooserOutputs.bUseMotionMatching)
	{
		FPoseSearchBlueprintResult PoseSearchResult;
		UPoseSearchLibrary::MotionMatch(this, Objects, AnimNames.PoseHistoryTag, FPoseSearchContinuingProperties(),
		                                FPoseSearchFutureProperties(), PoseSearchResult);

		SearchCost = PoseSearchResult.SearchCost;

		UAnimationAsset* AnimationAsset = static_cast<UAnimationAsset*>(PoseSearchResult.SelectedAnimation);

		const bool NoValidAnim = ChooserOutputs.MMCostLimit > 0.f
			                         ? PoseSearchResult.SearchCost <= ChooserOutputs.MMCostLimit
			                         : true;
		if (!IsValid(AnimationAsset) && NoValidAnim)
		{
			bNoValidAnim = true;
			return;
		}

		BlendStackInputs.AnimationAsset = AnimationAsset;
		UpdateAnimationLoopingFlag(BlendStackInputs);
		BlendStackInputs.StartTime = PoseSearchResult.SelectedTime;
	}

	if (bForceBlend)
	{
		EAnimNodeReferenceConversionResult Result{};
		const FBlendStackAnimNodeReference Reference{
			UBlendStackAnimNodeLibrary::ConvertToBlendStackNode(Node, Result)
		};
		if (Result == EAnimNodeReferenceConversionResult::Failed)
		{
			return;
		}
		UBlendStackAnimNodeLibrary::ForceBlendNextUpdate(Reference);
	}
}

bool UGASPAnimInstance::IsAnimationAlmostComplete()
{
	const float CurrentTime = GetInstanceAssetPlayerTime(0);
	const float Length = GetInstanceAssetPlayerLength(0);
	return !MotionMatching.bLoop && (Length - CurrentTime) <= 0.75f;
}

float UGASPAnimInstance::GetDynamicPlayRate(const FAnimNodeReference& Node) const
{
	static const FName EnablePlayRateWarpingCurveName = TEXT("Enable_PlayRateWarping");
	static const FName MoveDataSpeedCurveName = TEXT("MoveData_Speed");
	static const FName MaxDynamicPlayRateCurveName = TEXT("MaxDynamicPlayRate");
	static const FName MinDynamicPlayRateCurveName = TEXT("MinDynamicPlayRate");

	const UAnimSequence* AnimSequence{
		Cast<UAnimSequence>(UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAsset(Node))
	};
	if (!IsValid(AnimSequence))
	{
		return 0.f;
	}

	const float AnimTime = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAssetTime(Node);

	float AlphaCurve = 0.f;
	float SpeedCurve = 0.f;
	float MaxDynamicPlayRate = 1.25f;
	float MinDynamicPlayRate = 0.75f;

	if (!GetCurveValueSafe(AnimSequence, EnablePlayRateWarpingCurveName, AnimTime, AlphaCurve))
	{
		return 1.f;
	}

	if (!GetCurveValueSafe(AnimSequence, MoveDataSpeedCurveName, AnimTime, SpeedCurve))
	{
		return 1.f;
	}

	if (!GetCurveValueSafe(AnimSequence, MaxDynamicPlayRateCurveName, AnimTime, MaxDynamicPlayRate))
	{
		MaxDynamicPlayRate = 1.25f;
	}

	if (!GetCurveValueSafe(AnimSequence, MinDynamicPlayRateCurveName, AnimTime, MinDynamicPlayRate))
	{
		MinDynamicPlayRate = .75f;
	}


	const float SpeedRatio = SpeedCurve != 0.f ? CharacterInfo.Speed / SpeedCurve : 0.f;

	return FMath::Lerp(1.f, FMath::Clamp(SpeedRatio, MinDynamicPlayRate, MaxDynamicPlayRate), AlphaCurve);
}

void UGASPAnimInstance::OnStateEntryIdleLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	SetBlendStackAnimFromChooser(Node, EStateMachineState::IdleLoop);
}

void UGASPAnimInstance::OnStateEntryTransitionToIdleLoop(const FAnimUpdateContext& Context,
                                                         const FAnimNodeReference& Node)
{
	SetBlendStackAnimFromChooser(Node, EStateMachineState::TransitionToIdleLoop, true);
}

void UGASPAnimInstance::OnStateEntryLocomotionLoop(const FAnimUpdateContext& Context,
                                                   const FAnimNodeReference& Node)
{
	TargetRotationOnTransitionStart = TargetRotation;
	SetBlendStackAnimFromChooser(Node, EStateMachineState::LocomotionLoop);
}

void UGASPAnimInstance::OnStateEntryTransitionToLocomotionLoop(const FAnimUpdateContext& Context,
                                                               const FAnimNodeReference& Node)
{
	TargetRotationOnTransitionStart = TargetRotation;
	SetBlendStackAnimFromChooser(Node, EStateMachineState::TransitionToLocomotionLoop, true);
}

void UGASPAnimInstance::OnUpdateTransitionToLocomotionLoop(const FAnimUpdateContext& Context,
                                                           const FAnimNodeReference& Node)
{
	TargetRotationOnTransitionStart = FMath::RInterpTo(TargetRotationOnTransitionStart, TargetRotation,
	                                                   GetDeltaSeconds(), 5.f);
}

void UGASPAnimInstance::OnStateEntryInAirLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	SetBlendStackAnimFromChooser(Node, EStateMachineState::InAirLoop, false);
}

void UGASPAnimInstance::OnStateEntryTransitionToInAirLoop(const FAnimUpdateContext& Context,
                                                          const FAnimNodeReference& Node)
{
	SetBlendStackAnimFromChooser(Node, EStateMachineState::TransitionToInAirLoop, true);
}

void UGASPAnimInstance::OnStateEntryIdleBreak(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	SetBlendStackAnimFromChooser(Node, EStateMachineState::IdleBreak, true);
}

void UGASPAnimInstance::RefreshTargetRotation()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::RefreshTargetRotation"),
	                            STAT_UGASPAnimInstance_RefreshTargetRotation, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	if (MovementState.IsMoving())
	{
		switch (RotationMode)
		{
		case ERotationMode::Strafe:
			TargetRotation = CharacterInfo.ActorTransform.Rotator();
			TargetRotation.Yaw += GetStrafeYawRotationOffset();
			break;
		default:
			TargetRotation = CharacterInfo.ActorTransform.Rotator();
			break;
		}
	}
	else
	{
		TargetRotation = CharacterInfo.ActorTransform.Rotator();
	}

	TargetRotationDelta = (TargetRotation - CharacterInfo.RootTransform.Rotator()).GetNormalized().Yaw;
}

float UGASPAnimInstance::GetStrafeYawRotationOffset() const
{
	static const TMap<EMovementDirection, FName> CurveNames = {
		{EMovementDirection::B, TEXT("StrafeOffset_B")},
		{EMovementDirection::LL, TEXT("StrafeOffset_LL")},
		{EMovementDirection::LR, TEXT("StrafeOffset_LR")},
		{EMovementDirection::RL, TEXT("StrafeOffset_RL")},
		{EMovementDirection::RR, TEXT("StrafeOffset_RR")},
		{EMovementDirection::F, TEXT("StrafeOffset_F")}
	};

	const float Dir = FGASPMath::CalculateDirection(MotionMatching.FutureVelocity.GetSafeNormal(),
	                                                CharacterInfo.ActorTransform.Rotator());
	const float MappedDirection = FMath::GetMappedRangeValueClamped<float, float>({-180.f, 180.f},
		{0.f, 8.f}, Dir);

	const FName* CurveName{CurveNames.Find(MovementDirection)};
	if (!CurveName)
	{
		CurveName = CurveNames.Find(EMovementDirection::F);
	}

	float CurveValue;
	UAnimationWarpingLibrary::GetCurveValueFromAnimation(StrafeCurveAnimationAsset, *CurveName, MappedDirection,
	                                                     CurveValue);
	return CurveValue;
}

void UGASPAnimInstance::UpdateAnimationLoopingFlag(FGASPBlendStackInputs& Inputs) const
{
	if (Inputs.AnimationAsset.Get())
	{
		UPoseSearchLibrary::IsAnimationAssetLooping(Inputs.AnimationAsset.Get(), Inputs.bLoop);
	}
}

bool UGASPAnimInstance::GetCurveValueSafe(const UAnimSequence* AnimSequence, const FName& CurveName,
                                          const float Time,
                                          float& OutValue)
{
	return AnimSequence &&
		UAnimationWarpingLibrary::GetCurveValueFromAnimation(AnimSequence, CurveName, Time, OutValue);
}
