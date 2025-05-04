// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/GASPCharacter.h"
#include "MotionWarpingComponent.h"
#include "GameplayTagContainer.h"
#include "Components/GASPCharacterMovementComponent.h"
#include "Components/GASPTraversalComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASPCharacter)

// Sets default values
AGASPCharacter::AGASPCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGASPCharacterMovementComponent>(CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need
	// it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationRoll = bUseControllerRotationPitch = bUseControllerRotationYaw = false;

	SetReplicates(true);
	SetReplicatingMovement(true);

	if (GetMesh())
	{
		GetMesh()->bEnableUpdateRateOptimizations = false;
		GetMesh()->SetRelativeRotation_Direct({0.f, -90.f, 0.f});
		GetMesh()->SetRelativeLocation_Direct({0.f, 0.f, -90.f});
	}
	MovementComponent = Cast<UGASPCharacterMovementComponent>(GetCharacterMovement());
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
	TraversalComponent = CreateDefaultSubobject<UGASPTraversalComponent>(TEXT("TraversalComponent"));
}

// Called when the game starts or when spawned
void AGASPCharacter::BeginPlay()
{
	Super::BeginPlay();

	check(MovementComponent)

	SetGait(DesiredGait, true);
	SetRotationMode(RotationMode, true);
	SetOverlayMode(OverlayMode, true);
	SetLocomotionAction(FGameplayTag::EmptyTag, true);

	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		OnCharacterMovementUpdated.AddUniqueDynamic(this, &ThisClass::OnMovementUpdateSimulatedProxy);
	}
}

void AGASPCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OnCharacterMovementUpdated.RemoveAll(this);
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

// Called every frame
void AGASPCharacter::Tick(float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("AGASPCharacter::Tick"),
	                            STAT_AGASPCharacter_Tick, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	Super::Tick(DeltaTime);

	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		SetReplicatedAcceleration(MovementComponent->GetCurrentAcceleration());
	}

	const bool IsMoving = !MovementComponent->Velocity.IsNearlyZero(.1f) && !ReplicatedAcceleration.IsNearlyZero(10.f);
	SetMovementState(IsMoving ? EMovementState::Moving : EMovementState::Idle);

	if (LocomotionAction == LocomotionActionTags::Ragdoll)
	{
		RefreshRagdolling(DeltaTime);
	}

	if (MovementMode == MovementModeTags::Grounded)
	{
		if (StanceMode != StanceTags::Crouching)
		{
			RefreshGait();
		}
	}
}

void AGASPCharacter::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	MovementComponent = Cast<UGASPCharacterMovementComponent>(GetCharacterMovement());
}

void AGASPCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStanceMode(StanceTags::Crouching);
}

void AGASPCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStanceMode(StanceTags::Standing);
}

void AGASPCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	GetMesh()->AddTickPrerequisiteActor(this);

	MovementComponent = Cast<UGASPCharacterMovementComponent>(GetCharacterMovement());
}

void AGASPCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;

	// Replicate to everyone except owner
	Parameters.Condition = COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DesiredGait, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, RotationMode, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MovementMode, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MovementState, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, StanceMode, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, LocomotionAction, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedAcceleration, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, TraversalComponent, Parameters);

	// Replicate to everyone
	Parameters.Condition = COND_None;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OverlayMode, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, RagdollTargetLocation, Parameters);
}

void AGASPCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PrevCustomMode);

	if (!ensure(MovementComponent))
	{
		return;
	}

	if (MovementComponent->IsMovingOnGround())
	{
		SetMovementMode(MovementModeTags::Grounded);
	}
	else if (MovementComponent->IsFalling())
	{
		SetMovementMode(MovementModeTags::InAir);
	}
	else if (MovementComponent == MOVE_None)
	{
		SetMovementMode(FGameplayTag::EmptyTag);
	}
}

void AGASPCharacter::SetGait(const EGait NewGait, const bool bForce)
{
	if (!ensure(MovementComponent))
	{
		return;
	}

	if (NewGait != Gait || bForce)
	{
		const auto& OldGait{Gait};
		Gait = NewGait;
		MovementComponent->SetGait(NewGait);

		GaitChanged.Broadcast(OldGait);
	}
}

void AGASPCharacter::SetDesiredGait(EGait NewGait, bool bForce)
{
	if (NewGait != Gait || bForce)
	{
		DesiredGait = NewGait;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DesiredGait, this);

		if (GetLocalRole() == ROLE_AutonomousProxy && IsValid(GetNetConnection()))
		{
			Server_SetDesiredGait(NewGait);
		}
	}
}

void AGASPCharacter::SetRotationMode(const ERotationMode NewRotationMode, const bool bForce)
{
	if (NewRotationMode != RotationMode || bForce)
	{
		const auto& OldRotationMode{RotationMode};
		RotationMode = NewRotationMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, RotationMode, this);

		MovementComponent->SetRotationMode(NewRotationMode);

		if (GetLocalRole() == ROLE_AutonomousProxy && IsValid(GetNetConnection()))
		{
			Server_SetRotationMode(NewRotationMode);
		}

		RotationModeChanged.Broadcast(OldRotationMode);
	}
}

void AGASPCharacter::SetMovementMode(const FGameplayTag NewMovementMode, const bool bForce)
{
	if (NewMovementMode != MovementMode || bForce)
	{
		auto OldMovementMode{MovementMode};
		MovementMode = NewMovementMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MovementMode, this);
		if (GetLocalRole() == ROLE_AutonomousProxy && IsValid(GetNetConnection()))
		{
			Server_SetMovementMode(NewMovementMode);
		}

		MovementModeChanged.Broadcast(MovementMode);
	}
}

void AGASPCharacter::Server_SetMovementMode_Implementation(const FGameplayTag NewMovementMode)
{
	SetMovementMode(NewMovementMode);
}

void AGASPCharacter::Server_SetRotationMode_Implementation(const ERotationMode NewRotationMode)
{
	SetRotationMode(NewRotationMode);
}

void AGASPCharacter::Server_SetDesiredGait_Implementation(const EGait NewGait)
{
	SetDesiredGait(NewGait);
}

void AGASPCharacter::SetMovementState(const EMovementState NewMovementState, const bool bForce)
{
	if (!ensure(MovementComponent))
	{
		return;
	}

	if (NewMovementState != MovementState || bForce)
	{
		const auto& OldMovementState{MovementState};
		MovementState = NewMovementState;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MovementState, this);

		if (GetLocalRole() == ROLE_AutonomousProxy && IsValid(GetNetConnection()))
		{
			Server_SetMovementState(NewMovementState);
		}
		MovementStateChanged.Broadcast(OldMovementState);
	}
}

void AGASPCharacter::SetStanceMode(const FGameplayTag NewStanceMode, const bool bForce)
{
	if (StanceMode != NewStanceMode || bForce)
	{
		const auto& OldStanceMode{StanceMode};
		StanceMode = NewStanceMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, StanceMode, this);

		if (GetLocalRole() == ROLE_AutonomousProxy && IsValid(GetNetConnection()))
		{
			Server_SetStanceMode(NewStanceMode);
		}
		StanceModeChanged.Broadcast(OldStanceMode);
	}
}

void AGASPCharacter::Server_SetStanceMode_Implementation(const FGameplayTag NewStanceMode)
{
	SetStanceMode(NewStanceMode);
}

void AGASPCharacter::Server_SetMovementState_Implementation(const EMovementState NewMovementState)
{
	SetMovementState(NewMovementState);
}

void AGASPCharacter::MoveAction(const FVector2D& Value)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	const FRotator Rotation = GetControlRotation();
	const auto YawRotation = FRotator(0.f, Rotation.Yaw, 0.f);
	const FVector ForwardDirectionDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirectionDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector2D MovementInputScale = GetMovementInputScaleValue(Value);
	AddMovementInput(ForwardDirectionDirection, MovementInputScale.X);
	AddMovementInput(RightDirectionDirection, MovementInputScale.Y);
}

void AGASPCharacter::LookAction(const FVector2D& Value)
{
	AddControllerYawInput(Value.X);
	AddControllerPitchInput(-1 * Value.Y);
}

bool AGASPCharacter::CanSprint()
{
	if (RotationMode == ERotationMode::OrientToMovement)
	{
		return true;
	}
	if (RotationMode == ERotationMode::Aim)
	{
		return false;
	}

	const FVector ViewDirection{GetActorForwardVector()};

	const float Dot = FVector::DotProduct(ReplicatedAcceleration.GetSafeNormal2D(), ViewDirection.GetSafeNormal2D());

	return Dot > FMath::Cos(FMath::DegreesToRadians(50.f));
}

UGASPCharacterMovementComponent* AGASPCharacter::GetBCharacterMovement() const
{
	return static_cast<UGASPCharacterMovementComponent*>(GetCharacterMovement());
}

void AGASPCharacter::SetOverlayMode(const FGameplayTag NewOverlayMode, const bool bForce)
{
	if (NewOverlayMode != OverlayMode || bForce)
	{
		const auto& OldOverlayMode{OverlayMode};
		OverlayMode = NewOverlayMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OverlayMode, this);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetOverlayMode(NewOverlayMode);
		}
		OverlayModeChanged.Broadcast(OldOverlayMode);
	}
}

void AGASPCharacter::SetLocomotionAction(const FGameplayTag NewLocomotionAction, const bool bForce)
{
	if (NewLocomotionAction != LocomotionAction || bForce)
	{
		const auto& OldLocomotionAction{LocomotionAction};
		LocomotionAction = NewLocomotionAction;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LocomotionAction, this);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetLocomotionAction(NewLocomotionAction);
		}
		LocomotionActionChanged.Broadcast(OldLocomotionAction);
	}
}

void AGASPCharacter::Server_SetLocomotionAction_Implementation(FGameplayTag NewLocomotionAction)
{
	SetLocomotionAction(NewLocomotionAction);
}

void AGASPCharacter::Server_SetOverlayMode_Implementation(const FGameplayTag NewOverlayMode)
{
	SetOverlayMode(NewOverlayMode);
}

void AGASPCharacter::SetReplicatedAcceleration(const FVector& NewAcceleration)
{
	COMPARE_ASSIGN_AND_MARK_PROPERTY_DIRTY(ThisClass, ReplicatedAcceleration, NewAcceleration, this);
}

void AGASPCharacter::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
                                                      const FVector& PreviousFloorContactNormal,
                                                      const FVector& PreviousLocation, float TimeDelta)
{
	Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation,
	                                        TimeDelta);
	UnCrouch();
}

void AGASPCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	const float VolumeMultiplier = FMath::GetMappedRangeValueClamped<float, float>(
		{0.f, 500.f}, {.2f, 1.5f}, GetVelocity().Size2D());
	PlayAudioEvent(FoleyJumpTag, VolumeMultiplier);
}

void AGASPCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	const float VolumeMultiplier = FMath::GetMappedRangeValueClamped<float, float>(
		{-500.f, -900.f}, {.0f, 1.5f}, GetVelocity().Z);
	PlayAudioEvent(FoleyLandTag, VolumeMultiplier);
}

bool AGASPCharacter::HasFullMovementInput() const
{
	if (MovementStickMode == EAnalogStickBehaviorMode::FixedWalkRun || MovementStickMode == EAnalogStickBehaviorMode::VariableWalkRun)
	{
		return GetPendingMovementInputVector().Size2D() >= AnalogMovementThreshold;
	}

	return true;
}

FVector2D AGASPCharacter::GetMovementInputScaleValue(const FVector2D InVector) const
{
	return MovementStickMode > EAnalogStickBehaviorMode::FixedWalkRun ? InVector : InVector.GetSafeNormal();
}

void AGASPCharacter::RefreshGait()
{
	EGait NewGait;

	if (DesiredGait == EGait::Sprint && CanSprint())
	{
		NewGait = HasFullMovementInput() ? EGait::Sprint : EGait::Run;
	}
	else if (DesiredGait == EGait::Walk)
	{
		NewGait = EGait::Walk;
	}
	else
	{
		NewGait = HasFullMovementInput() ? EGait::Run : EGait::Walk;
	}

	SetGait(NewGait);
}

void AGASPCharacter::OnMovementUpdateSimulatedProxy_Implementation(float DeltaSeconds, FVector OldLocation,
                                                                   const FVector OldVelocity)
{
	if (MovementMode != PreviousMovementMode)
	{
		if (float VolumeMultiplier; MovementMode == MovementModeTags::Grounded)
		{
			VolumeMultiplier = FMath::GetMappedRangeValueClamped<float, float>(
				{-500.f, -900.f}, {.2f, 1.5f}, OldVelocity.Z);
			PlayAudioEvent(FoleyJumpTag, VolumeMultiplier);
		}
		else if (MovementMode == MovementModeTags::InAir)
		{
			VolumeMultiplier = FMath::GetMappedRangeValueClamped<float, float>(
				{.0f, 500.f}, {.2f, 1.f}, OldVelocity.Size2D());
			PlayAudioEvent(FoleyLandTag, VolumeMultiplier);
		}
	}
	PreviousMovementMode = MovementMode;
}

FTraversalResult AGASPCharacter::TryTraversalAction() const
{
	if (IsValid(TraversalComponent))
	{
		return TraversalComponent->TryTraversalAction(GetTraversalCheckInputs());
	}

	return {true, false};
}

bool AGASPCharacter::IsDoingTraversal() const
{
	return IsValid(TraversalComponent) && TraversalComponent->IsDoingTraversal();
}

FTraversalCheckInputs AGASPCharacter::GetTraversalCheckInputs() const
{
	const FVector ForwardVector{GetActorForwardVector()};
	if (MovementMode == MovementModeTags::InAir)
	{
		return {
			ForwardVector, 75.f, FVector::ZeroVector,
			{0.f, 0.f, 50.f}, 30.f, 86.f
		};
	}

	const FVector RotationVector = GetActorRotation().UnrotateVector(MovementComponent->Velocity);
	const float ClampedDistance = FMath::GetMappedRangeValueClamped<float, float>(
		{0.f, 500.f}, {75.f, 350.f}, RotationVector.X);

	return {
		ForwardVector, ClampedDistance, FVector::ZeroVector,
		FVector::ZeroVector, 30.f, 60.f
	};
}
