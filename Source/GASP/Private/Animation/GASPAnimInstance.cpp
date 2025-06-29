// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/GASPAnimInstance.h"
#include "Actors/GASPCharacter.h"
#include "Components/GASPCharacterMovementComponent.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "Utils/GASPMath.h"
#include "PoseSearch/MotionMatchingAnimNodeLibrary.h"
#include "ChooserFunctionLibrary.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "AnimationWarpingLibrary.h"
#include "BlendStack/BlendStackAnimNodeLibrary.h"
#include "BoneControllers/AnimNode_FootPlacement.h"
#include "Interfaces/GASPHeldObjectInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASPAnimInstance)

void UGASPAnimInstance::OnLanded(const FHitResult& HitResult)
{
	bLanded = true;

	GetWorld()->GetTimerManager().SetTimer(LandedHandle, FTimerDelegate::CreateLambda([this]()
	{
		bLanded = false;
	}), .3f, false);
}

EPoseSearchInterruptMode UGASPAnimInstance::GetMatchingInterruptMode() const
{
	return MovementMode != PreviousMovementMode || (MovementMode == MovementModeTags::Grounded && (
		       MovementState != PreviousMovementState || (Gait != PreviousGait && IsMoving()) || StanceMode !=
		       PreviousStanceMode))
		       ? EPoseSearchInterruptMode::InterruptOnDatabaseChange
		       : EPoseSearchInterruptMode::DoNotInterrupt;
}

EOffsetRootBoneMode UGASPAnimInstance::GetOffsetRootRotationMode() const
{
	if (IsSlotActive(AnimNames.AnimationSlotName))
	{
		return EOffsetRootBoneMode::Release;
	}

	float YawDifference = CharacterInfo.ActorTransform.Rotator().Yaw - CharacterInfo.RootTransform.Rotator().Yaw;
	YawDifference = FRotator::NormalizeAxis(YawDifference);

	return !IsMoving() && RotationMode == ERotationMode::Aim && FMath::Abs(YawDifference) >= 90.f
		       ? EOffsetRootBoneMode::LockOffsetAndConsumeAnimation
		       : EOffsetRootBoneMode::Accumulate;
}

EOffsetRootBoneMode UGASPAnimInstance::GetOffsetRootTranslationMode() const
{
	if (IsSlotActive(AnimNames.AnimationSlotName) || MovementMode == MovementModeTags::InAir || (!IsMoving() &&
		RotationMode == ERotationMode::Aim))
	{
		return EOffsetRootBoneMode::Release;
	}

	return MovementMode == MovementModeTags::Grounded && IsMoving()
		       ? EOffsetRootBoneMode::Interpolate
		       : EOffsetRootBoneMode::Release;
}

float UGASPAnimInstance::GetOffsetRootTranslationHalfLife() const
{
	return IsMoving() ? .3f : .1f;
}

EOrientationWarpingSpace UGASPAnimInstance::GetOrientationWarpingSpace() const
{
	return bOffsetRootBoneEnabled
		       ? EOrientationWarpingSpace::RootBoneTransform
		       : EOrientationWarpingSpace::ComponentTransform;
}

float UGASPAnimInstance::GetAOYaw() const
{
	if (!CachedCharacter.IsValid())
	{
		return 0.f;
	}

	return RotationMode == ERotationMode::OrientToMovement ? 0.f : GetAOValue().X;
}

FTransform UGASPAnimInstance::GetHandIKTransform(const FName HandIKSocketName, const FName ObjectIKSocketName,
                                                 const FVector& SocketOffset) const
{
	const auto* SkelMeshComp = GetSkelMeshComponent();
	if (!SkelMeshComp || !CachedCharacter.IsValid())
	{
		return FTransform::Identity;
	}

	const FTransform SocketTransform = SkelMeshComp->GetSocketTransform(HandIKSocketName);
	const auto* Interface = Cast<IGASPHeldObjectInterface>(CachedCharacter.Get());
	if (!Interface && !CachedCharacter->Implements<UGASPHeldObjectInterface>())
	{
		return FTransform::Identity;
	}

	const auto* HeldObject = Interface->Execute_GetHeldObject(CachedCharacter.Get());
	if (!IsValid(HeldObject) || !HeldObject->DoesSocketExist(ObjectIKSocketName))
	{
		return FTransform::Identity;
	}

	const FTransform ObjectTransform = HeldObject->GetSocketTransform(ObjectIKSocketName);
	return ObjectTransform.GetRelativeTransform(SocketTransform * FTransform(SocketOffset));
}

bool UGASPAnimInstance::IsEnableSteering() const
{
	return IsMoving() || MovementMode == MovementModeTags::InAir;
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
	CachedCharacter->OverlayModeChanged.AddUniqueDynamic(this, &ThisClass::OnOverlayChanged);
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
	RotationMode = CachedCharacter->GetRotationMode();
	MovementState = CachedCharacter->GetMovementState();
	LocomotionAction = CachedCharacter->GetLocomotionAction();
	MovementMode = CachedCharacter->GetMovementMode();
	StanceMode = CachedCharacter->GetStanceMode();

	StateContainer.RemoveTag(PreviousMovementMode);
	StateContainer.AddTagFast(CachedCharacter->GetMovementMode());
	StateContainer.RemoveTag(PreviousStanceMode);
	StateContainer.AddTagFast(CachedCharacter->GetStanceMode());
	StateContainer.RemoveTag(PreviousMovementState);
	StateContainer.AddTagFast(CachedCharacter->GetMovementState());

	RefreshEssentialValues(DeltaSeconds);
	RefreshTrajectory(DeltaSeconds);
	RefreshOverlaySettings(DeltaSeconds);
	RefreshLayering(DeltaSeconds);
	RefreshMovementDirection(DeltaSeconds);
	RefreshTargetRotation();

	if (LocomotionAction == LocomotionActionTags::Ragdoll)
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
	PreviousRotationMode = RotationMode;
	PreviousMovementState = MovementState;
	PreviousLocomotionAction = LocomotionAction;
	PreviousMovementMode = MovementMode;
	PreviousStanceMode = StanceMode;
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
	FTransformTrajectory OutTrajectory{};
	UPoseSearchTrajectoryLibrary::PoseSearchGenerateTransformTrajectory(this, TrajectoryData, DeltaSeconds, Trajectory,
	                                                                    BlendStack.PreviousDesiredYawRotation,
	                                                                    OutTrajectory,
	                                                                    -1.f, 30, .1f, 15);

	const TArray<AActor*> IgnoredActors{};
	UPoseSearchTrajectoryLibrary::HandleTransformTrajectoryWorldCollisions(
		CachedCharacter.Get(), this, OutTrajectory, true, .01f,
		Trajectory, CollisionResults, TraceTypeQuery_MAX, false,
		IgnoredActors, EDrawDebugTrace::None, true, 150.f);

	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, -.3f, -.2f, BlendStack.PreviousVelocity,
	                                                             false);
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, .0f, .2f, BlendStack.CurrentVelocity,
	                                                             false);
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, .4f, .5f, BlendStack.FutureVelocity,
	                                                             false);
}

void UGASPAnimInstance::RefreshMovementDirection(float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::MovementDirection"),
	                            STAT_UGASPAnimInstance_MovementDirection, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	PreviousMovementDirection = MovementDirection;
	if (!IsMoving())
	{
		return;
	}

	CharacterInfo.AngleVelocity = FGASPMath::CalculateDirection(CharacterInfo.Velocity.GetSafeNormal(),
	                                                            CharacterInfo.ActorTransform.Rotator());

	MovementDirection = FGASPMath::GetMovementDirection(CharacterInfo.AngleVelocity, 60.f, 5.f);
}

float UGASPAnimInstance::GetMatchingBlendTime() const
{
	if (MovementMode == MovementModeTags::InAir)
	{
		return CharacterInfo.Velocity.Z > 100.f ? .15f : .5f;
	}

	return PreviousMovementMode == MovementModeTags::Grounded ? .5f : .2f;
}

FFloatInterval UGASPAnimInstance::GetMatchingPlayRate() const
{
	if (MovementMode == MovementModeTags::Grounded)
	{
		return {.5f, 3.f};
	}
	return {.75f, 1.25f};
}

FFootPlacementPlantSettings UGASPAnimInstance::GetPlantSettings() const
{
	return BlendStack.DatabaseTags.Contains(AnimNames.StopsTag)
		       ? PlantSettings_Stops
		       : PlantSettings_Default;
}

FFootPlacementInterpolationSettings UGASPAnimInstance::GetPlantInterpolationSettings() const
{
	return BlendStack.DatabaseTags.Contains(AnimNames.StopsTag)
		       ? InterpolationSettings_Stops
		       : InterpolationSettings_Default;
}

float UGASPAnimInstance::GetMatchingNotifyRecencyTimeOut() const
{
	static constexpr float RecencyTimeOuts[]{.2f, .2f, .16f};

	return Gait >= EGait::Walk && Gait <= EGait::Sprint ? RecencyTimeOuts[static_cast<int32>(Gait)] : .2f;
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
	return BlendStack.FutureVelocity.Size2D() > CharacterInfo.Velocity.Size2D() + 100.f && !BlendStack.
		DatabaseTags.Contains(AnimNames.PivotsTag) && IsMoving() && CachedMovement.IsValid();
}

bool UGASPAnimInstance::IsPivoting() const
{
	if (BlendStack.FutureVelocity.IsNearlyZero() || CharacterInfo.Velocity.IsNearlyZero())
	{
		return false;
	}

	if (!bUseExperimentalStateMachine)
	{
		return FMath::Abs(GetTrajectoryTurnAngle()) >= (RotationMode == ERotationMode::OrientToMovement ? 45.f : 30.f);
	}

	auto InRange = [this](const float Speed)
	{
		if (StanceMode == StanceTags::Crouching)
		{
			return Speed > 50.f && Speed < 200.f;
		}
		static const TMap<EGait, FVector2D> GaitRanges = {
			{EGait::Walk, {50.f, 200.f}},
			{EGait::Run, {200.f, 550.f}},
			{EGait::Sprint, {200.f, 700.f}},
		};

		const FVector2D* MinMax = GaitRanges.Find(Gait);
		return Speed > MinMax->X && Speed < MinMax->Y;
	};

	auto ClampedSpeed = [this](const float Speed)
	{
		if (StanceMode == StanceTags::Crouching)
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
		IsMoving();
}

bool UGASPAnimInstance::IsMoving() const
{
	return MovementState == MovementStateTags::Moving;
}

bool UGASPAnimInstance::ShouldTurnInPlace() const
{
	float YawDifference = CharacterInfo.ActorTransform.Rotator().Yaw - CharacterInfo.RootTransform.Rotator().Yaw;
	YawDifference = FRotator::NormalizeAxis(YawDifference);

	return FMath::Abs(YawDifference) >= CharacterInfo.MaxTurnAngle && RotationMode == ERotationMode::Aim &&
		!IsMoving();
}

bool UGASPAnimInstance::ShouldSpin() const
{
	float YawDifference = CharacterInfo.ActorTransform.Rotator().Yaw - CharacterInfo.RootTransform.Rotator().Yaw;
	YawDifference = FRotator::NormalizeAxis(YawDifference);

	return FMath::Abs(YawDifference) >= 130.f && CharacterInfo.Speed >= 150.f && !BlendStack.DatabaseTags.Contains(
		AnimNames.PivotsTag);
}

bool UGASPAnimInstance::JustLanded_Light() const
{
	return FMath::Abs(PreviousCharacterInfo.Velocity.Z) < FMath::Abs(HeavyLandSpeedThreshold) && bLanded;
}

bool UGASPAnimInstance::JustLanded_Heavy() const
{
	return FMath::Abs(PreviousCharacterInfo.Velocity.Z) >= FMath::Abs(HeavyLandSpeedThreshold) && bLanded;
}

bool UGASPAnimInstance::JustTraversed() const
{
	return !IsSlotActive(AnimNames.AnimationSlotName) && GetCurveValue(AnimNames.MovingTraversalCurveName) > 0.f &&
		GetTrajectoryTurnAngle() <= 50.f;
}

bool UGASPAnimInstance::PlayLand() const
{
	return MovementMode == MovementModeTags::Grounded && PreviousMovementMode == MovementModeTags::InAir;
}

bool UGASPAnimInstance::PlayMovingLand() const
{
	return MovementMode == MovementModeTags::Grounded && PreviousMovementMode == MovementModeTags::InAir &&
		FMath::Abs(GetTrajectoryTurnAngle()) <= 120.f;
}

float UGASPAnimInstance::GetTrajectoryTurnAngle() const
{
	const FVector2D CurrentVelocity2D(CharacterInfo.Velocity.X, CharacterInfo.Velocity.Y);
	const FVector2D FutureVelocity2D(BlendStack.FutureVelocity.X, BlendStack.FutureVelocity.Y);

	const float CurrentAngle = FMath::Atan2(CurrentVelocity2D.Y, CurrentVelocity2D.X);
	const float FutureAngle = FMath::Atan2(FutureVelocity2D.Y, FutureVelocity2D.X);

	const float DeltaAngle = FMath::RadiansToDegrees(FutureAngle - CurrentAngle);

	return FMath::Fmod(DeltaAngle + 180.0f, 360.0f) - 180.0f;
}

FVector2D UGASPAnimInstance::GetLeanAmount() const
{
	if (!CachedCharacter.IsValid())
	{
		return FVector2D::ZeroVector;
	}

	const FVector RelAccel{GetRelativeAcceleration()};
	return FVector2D(
		RelAccel * FMath::GetMappedRangeValueClamped<float, float>({200.f, 500.f}, {.5f, 1.f}, CharacterInfo.Speed));
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

	const auto Objects{
		UChooserFunctionLibrary::EvaluateChooserMulti(this, LocomotionTable,
		                                              UPoseSearchDatabase::StaticClass())
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
	BlendStack.PoseSearchDatabase = OutResult.SelectedDatabase;
}

void UGASPAnimInstance::RefreshOffsetRoot(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!bOffsetRootBoneEnabled)
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
	const auto DesiredFacing = FQuat(TargetRotation);
	const auto Offset = FQuat({0.f, -90.f, 0.f});
	return DesiredFacing * Offset;
}

void UGASPAnimInstance::RefreshBlendStack(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::RefreshBlendStack"),
	                            STAT_UGASPAnimInstance_RefreshBlendStack, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	BlendStack.AnimTime = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAssetTime(Node);
	BlendStack.AnimAsset = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAsset(Node);
	BlendStack.PlayRate = GetDynamicPlayRate(Node);
	const UAnimSequence* NewAnimSequence{static_cast<UAnimSequence*>(BlendStack.AnimAsset.Get())};
	if (!NewAnimSequence)
	{
		return;
	}

	UAnimationWarpingLibrary::GetCurveValueFromAnimation(NewAnimSequence, AnimNames.EnableMotionWarpingCurveName,
	                                                     BlendStack.AnimTime, BlendStack.OrientationAlpha);
}

void UGASPAnimInstance::RefreshBlendStackMachine(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::RefreshBlendStackMachine"),
	                            STAT_UGASPAnimInstance_RefreshBlendStackMachine, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	EAnimNodeReferenceConversionResult Result{};
	const FBlendStackAnimNodeReference Reference{
		UBlendStackAnimNodeLibrary::ConvertToBlendStackNode(Node, Result)
	};
	if (Result == EAnimNodeReferenceConversionResult::Failed)
	{
		return;
	}
	BlendStackMachine.bLoop = UBlendStackAnimNodeLibrary::IsCurrentAssetLooping(Reference);
	BlendStackMachine.AssetTimeRemaining = UBlendStackAnimNodeLibrary::GetCurrentAssetTimeRemaining(Reference);
}

void UGASPAnimInstance::RefreshEssentialValues(const float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::RefreshEssentialValues"),
	                            STAT_UGASPAnimInstance_RefreshEssentialValues, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);
	CharacterInfo.ActorTransform = CachedCharacter->GetActorTransform();

	if (!bOffsetRootBoneEnabled)
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

	if (IsMoving())
	{
		BlendStack.LastNonZeroVector = CharacterInfo.Velocity;
	}
}

void UGASPAnimInstance::RefreshRagdollValues(const float DeltaSeconds)
{
	static constexpr auto ReferenceSpeed{1000.0f};
	RagdollingState.FlailPlayRate = FMath::Clamp(
		UE_REAL_TO_FLOAT(CachedCharacter->GetRagdollingState().Velocity.Size() / ReferenceSpeed), 0.f, 1.f);
}

bool UGASPAnimInstance::IsEnabledAO() const
{
	return FMath::Abs(GetAOValue().X) <= 115.f && RotationMode != ERotationMode::OrientToMovement &&
		GetSlotMontageLocalWeight(AnimNames.AnimationSlotName) < .5f;
}

FVector2D UGASPAnimInstance::GetAOValue() const
{
	if (!CachedCharacter.IsValid())
	{
		return FVector2D::ZeroVector;
	}

	const FRotator ControlRot = CachedCharacter->IsLocallyControlled()
		                            ? CachedCharacter->GetControlRotation()
		                            : CachedCharacter->GetBaseAimRotation();

	const FRotator RootRot = CharacterInfo.RootTransform.Rotator();
	FRotator DeltaRot = (ControlRot - RootRot).GetNormalized();

	const float DisableBlend = GetCurveValue(AnimNames.DisableAOCurveName);
	return FMath::Lerp({DeltaRot.Yaw, DeltaRot.Pitch}, FVector2D::ZeroVector, DisableBlend);
}

bool UGASPAnimInstance::CanOverlayTransition() const
{
	return StanceMode == StanceTags::Standing && !IsMoving();
}

void UGASPAnimInstance::RefreshOverlaySettings(float DeltaTime)
{
	const float ClampedYawAxis = FMath::ClampAngle(GetAOValue().X, -90.f, 90.f) / 6.f;
	SpineRotation.Yaw = FMath::FInterpTo(SpineRotation.Yaw, ClampedYawAxis, DeltaTime, 60.f);
}

void UGASPAnimInstance::RefreshLayering(float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGASPAnimInstance::RefreshLayering"),
	                            STAT_UGASPAnimInstance_RefreshLayering, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	LayeringState.SpineAdditiveBlendAmount = GetCurveValue(AnimNames.LayeringSpineAdditiveName);
	LayeringState.HeadAdditiveBlendAmount = GetCurveValue(AnimNames.LayeringHeadAdditiveName);

	LayeringState.ArmLeftAdditiveBlendAmount = GetCurveValue(AnimNames.LayeringArmLeftAdditiveName);
	LayeringState.ArmRightAdditiveBlendAmount = GetCurveValue(AnimNames.LayeringArmRightAdditiveName);

	LayeringState.HandLeftBlendAmount = GetCurveValue(AnimNames.LayeringHandLeftName);
	LayeringState.HandRightBlendAmount = GetCurveValue(AnimNames.LayeringHandRightName);

	LayeringState.EnableHandLeftIKBlend = FMath::Lerp(0.f, GetCurveValue(AnimNames.LayeringHandLeftIKName),
	                                                  GetCurveValue(AnimNames.LayeringArmLeftName));
	LayeringState.EnableHandRightIKBlend = FMath::Lerp(0.f, GetCurveValue(AnimNames.LayeringHandRightIKName),
	                                                   GetCurveValue(AnimNames.LayeringArmRightName));

	LayeringState.ArmLeftLocalSpaceBlendAmount = GetCurveValue(AnimNames.LayeringArmLeftLocalSpaceName);
	LayeringState.ArmLeftMeshSpaceBlendAmount = UE_REAL_TO_FLOAT(
		1.f - FMath::FloorToInt(LayeringState.ArmLeftLocalSpaceBlendAmount));

	LayeringState.ArmRightLocalSpaceBlendAmount = GetCurveValue(AnimNames.LayeringArmRightLocalSpaceName);
	LayeringState.ArmRightMeshSpaceBlendAmount = UE_REAL_TO_FLOAT(
		1.f - FMath::FloorToInt(LayeringState.ArmRightLocalSpaceBlendAmount));

	BlendPoses.BasePoseN = FMath::FInterpTo(BlendPoses.BasePoseN,
	                                        StanceMode == StanceTags::Standing ? 1.f : 0.f,
	                                        DeltaTime, 15.f);
	BlendPoses.BasePoseCLF = FMath::GetMappedRangeValueClamped<float, float>(
		{0.f, 1.f}, {1.f, 0.f}, BlendPoses.BasePoseN);
}

void UGASPAnimInstance::SetBlendStackAnimFromChooser(const FAnimNodeReference& Node, const EStateMachineState NewState,
                                                     const bool bForceBlend)
{
	StateMachineState = NewState;
	PreviousBlendStackInputs = BlendStackInputs;

	bNoValidAnim = false;
	bNotifyTransition_ReTransition = false;
	bNotifyTransition_ToLoop = false;

	if (!StateMachineTable)
	{
		return;
	}

	FGASPChooserOutputs ChooserOutputs;
	FChooserEvaluationContext Context = UChooserFunctionLibrary::MakeChooserEvaluationContext();
	Context.AddObjectParam(this);
	Context.AddStructParam(ChooserOutputs);

	const auto Objects{
		UChooserFunctionLibrary::EvaluateObjectChooserBaseMulti(
			Context, UChooserFunctionLibrary::MakeEvaluateChooser(StateMachineTable),
			UAnimationAsset::StaticClass())
	};

	if (Objects.IsEmpty())
	{
		bNoValidAnim = true;
		return;
	}

	// Update blend stack inputs
	BlendStackInputs.AnimationAsset = static_cast<UAnimationAsset*>(Objects[0]);
	UpdateAnimationLoopingFlag(BlendStackInputs);
	BlendStackInputs.StartTime = ChooserOutputs.StartTime;
	BlendStackInputs.BlendTime = ChooserOutputs.BlendTime;
	BlendStackInputs.BlendProfile = GetBlendProfileByName(ChooserOutputs.BlendProfile);
	BlendStack.DatabaseTags = ChooserOutputs.Tags;

	if (ChooserOutputs.bUseMotionMatching)
	{
		FPoseSearchBlueprintResult PoseSearchResult;
		UPoseSearchLibrary::MotionMatch(this, Objects, AnimNames.PoseHistoryTag, FPoseSearchContinuingProperties(),
		                                FPoseSearchFutureProperties(), PoseSearchResult);

		SearchCost = PoseSearchResult.SearchCost;

		auto AnimationAsset = static_cast<UAnimationAsset*>(PoseSearchResult.SelectedAnim);

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

bool UGASPAnimInstance::IsAnimationAlmostComplete() const
{
	return !BlendStackMachine.bLoop && BlendStackMachine.AssetTimeRemaining <= .75f;
}

float UGASPAnimInstance::GetDynamicPlayRate(const FAnimNodeReference& Node) const
{
	static const FName EnablePlayRateWarpingCurveName = TEXT("Enable_PlayRateWarping");
	static const FName MoveDataSpeedCurveName = TEXT("MoveData_Speed");
	static const FName MaxDynamicPlayRateCurveName = TEXT("MaxDynamicPlayRate");
	static const FName MinDynamicPlayRateCurveName = TEXT("MinDynamicPlayRate");

	const UAnimSequence* AnimSequence{
		static_cast<UAnimSequence*>(UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAsset(Node))
	};
	if (!IsValid(AnimSequence))
	{
		return 0.f;
	}

	const float AnimTime = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAssetTime(Node);

	float AlphaCurve{0.f};
	float SpeedCurve{0.f};
	float MaxDynamicPlayRate;
	float MinDynamicPlayRate;

	if (!UAnimationWarpingLibrary::GetCurveValueFromAnimation(AnimSequence, EnablePlayRateWarpingCurveName, AnimTime,
	                                                          AlphaCurve))
	{
		return 1.f;
	}

	if (!UAnimationWarpingLibrary::GetCurveValueFromAnimation(AnimSequence, MoveDataSpeedCurveName, AnimTime,
	                                                          SpeedCurve))
	{
		return 1.f;
	}

	if (!UAnimationWarpingLibrary::GetCurveValueFromAnimation(AnimSequence, MaxDynamicPlayRateCurveName, AnimTime,
	                                                          MaxDynamicPlayRate))
	{
		MaxDynamicPlayRate = 3.f;
	}

	if (!UAnimationWarpingLibrary::GetCurveValueFromAnimation(AnimSequence, MinDynamicPlayRateCurveName, AnimTime,
	                                                          MinDynamicPlayRate))
	{
		MinDynamicPlayRate = .5f;
	}

	const float SpeedRatio = CharacterInfo.Speed / FMath::Clamp(SpeedCurve, 1.f, UE_MAX_FLT);

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

	if (IsMoving())
	{
		switch (RotationMode)
		{
		case ERotationMode::OrientToMovement:
			TargetRotation = CharacterInfo.ActorTransform.Rotator();
			break;
		default:
			TargetRotation = CharacterInfo.ActorTransform.Rotator();
			TargetRotation.Yaw += GetStrafeYawRotationOffset();
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
	if (!IsValid(StrafeCurveAnimationAsset))
	{
		return 0.f;
	}

	static const TMap<EMovementDirection, FName> CurveNames = {
		{EMovementDirection::B, TEXT("StrafeOffset_B")},
		{EMovementDirection::LL, TEXT("StrafeOffset_LL")},
		{EMovementDirection::LR, TEXT("StrafeOffset_LR")},
		{EMovementDirection::RL, TEXT("StrafeOffset_RL")},
		{EMovementDirection::RR, TEXT("StrafeOffset_RR")},
		{EMovementDirection::F, TEXT("StrafeOffset_F")}
	};

	const float Dir = FGASPMath::CalculateDirection(BlendStack.FutureVelocity.GetSafeNormal(),
	                                                CharacterInfo.ActorTransform.Rotator());
	const float MappedDirection = FMath::GetMappedRangeValueClamped<float, float>({-180.f, 180.f},
		{0.f, 8.f}, Dir) / 30.f;

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
