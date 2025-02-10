// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/GASPCharacter.h"

#include "GameplayTagContainer.h"
#include "Components/GASPCharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
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

	bReplicates = true;
	SetReplicatingMovement(true);

	GetMesh()->bEnableUpdateRateOptimizations = false;
	MovementComponent = Cast<UGASPCharacterMovementComponent>(GetCharacterMovement());
}

void AGASPCharacter::OnSoundsPreloaded()
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Display, TEXT("Sound asset preloaded"))
#endif
}

// Called when the game starts or when spawned
void AGASPCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(MovementComponent))
	{
		return;
	}

	SetGait(DesiredGait, true);
	SetRotationMode(DesiredRotationMode, true);
	SetOverlayState(OverlayState, true);
	SetLocomotionAction(ELocomotionAction::None, true);

	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		OnCharacterMovementUpdated.AddUniqueDynamic(this, &ThisClass::OnMovementUpdateSimulatedProxy);
	}
}

void AGASPCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (FoleyAudioBank)
	{
		FoleyAudioBank->OnSoundsPreloaded.RemoveDynamic(this, &ThisClass::OnSoundsPreloaded);
	}
}

// Called every frame
void AGASPCharacter::Tick(float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("AGASPCharacterTick"),
	                            STAT_AGASPCharacter_Tick, STATGROUP_GASP)
	TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__);

	Super::Tick(DeltaTime);

	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		SetReplicatedAcceleration(MovementComponent->GetCurrentAcceleration());
	}

	const bool IsMoving = !MovementComponent->Velocity.IsNearlyZero(.1f) && !ReplicatedAcceleration.IsZero();
	SetMovementState(IsMoving ? EMovementState::Moving : EMovementState::Idle);

	if (LocomotionAction == ELocomotionAction::Ragdoll)
	{
		RefreshRagdolling(DeltaTime);
	}

	if (MovementMode == ECMovementMode::OnGround)
	{
		UE_LOG(LogTemp, Display, TEXT("OnGround tick"));
		if (StanceMode != EStanceMode::Crouch)
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

	SetStanceMode(EStanceMode::Crouch);
}

void AGASPCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStanceMode(EStanceMode::Stand);
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

	DOREPLIFETIME(ThisClass, RagdollTargetLocation);

	// Replicate to everyone
	Parameters.Condition = COND_None;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OverlayState, Parameters);
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
		SetMovementMode(ECMovementMode::OnGround);
	}
	else if (MovementComponent->IsFalling())
	{
		SetMovementMode(ECMovementMode::InAir);
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
		Gait = NewGait;
		// MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Gait, this);

		MovementComponent->SetGait(NewGait);

		// if (GetLocalRole() == ROLE_AutonomousProxy && IsValid(GetNetConnection()))
		// {
		// 	Server_SetGait(NewGait);
		// }

		GaitChanged.Broadcast(NewGait);
	}
}

void AGASPCharacter::SetDesiredGait(EGait NewGait, bool bForce)
{
	if (!ensure(MovementComponent))
	{
		return;
	}

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
	if (!ensure(MovementComponent))
	{
		return;
	}

	if (NewRotationMode != RotationMode || bForce)
	{
		RotationMode = NewRotationMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, RotationMode, this);

		MovementComponent->SetRotationMode(NewRotationMode);

		if (GetLocalRole() == ROLE_AutonomousProxy && IsValid(GetNetConnection()))
		{
			Server_SetRotationMode(NewRotationMode);
		}

		RotationModeChanged.Broadcast(NewRotationMode);
	}
}

void AGASPCharacter::SetMovementMode(const ECMovementMode NewMovementMode, const bool bForce)
{
	if (!ensure(MovementComponent))
	{
		return;
	}

	if (NewMovementMode != MovementMode || bForce)
	{
		MovementMode = NewMovementMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MovementMode, this);
		if (GetLocalRole() == ROLE_AutonomousProxy && IsValid(GetNetConnection()))

		{
			Server_SetMovementMode(NewMovementMode);
		}
	}
}

void AGASPCharacter::Server_SetMovementMode_Implementation(const ECMovementMode NewMovementMode)
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
		MovementState = NewMovementState;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MovementState, this);

		if (GetLocalRole() == ROLE_AutonomousProxy && IsValid(GetNetConnection()))
		{
			Server_SetMovementState(NewMovementState);
		}
		MovementStateChanged.Broadcast(NewMovementState);
	}
}

void AGASPCharacter::SetStanceMode(EStanceMode NewStanceMode, bool bForce)
{
	if (StanceMode != NewStanceMode || bForce)
	{
		StanceMode = NewStanceMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, StanceMode, this);

		UE_LOG(LogTemp, Warning, TEXT("Crouch"))

		if (GetLocalRole() == ROLE_AutonomousProxy && IsValid(GetNetConnection()))
		{
			Server_SetStanceMode(NewStanceMode);
		}
		StanceModeChanged.Broadcast(NewStanceMode);
	}
}

void AGASPCharacter::Server_SetStanceMode_Implementation(const EStanceMode NewStanceMode)
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
	const FRotator YawRotation = FRotator(0.f, Rotation.Yaw, 0.f);
	const FVector ForwardDirectionDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirectionDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirectionDirection, Value.X);
	AddMovementInput(RightDirectionDirection, Value.Y);
}

void AGASPCharacter::LookAction(const FVector2D& Value)
{
	AddControllerYawInput(Value.Y);
	AddControllerPitchInput(-1 * Value.X);
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

	const FVector Acceleration{MovementComponent->GetCurrentAcceleration()};

	FRotator DeltaRot = GetActorRotation() - FRotationMatrix::MakeFromX(Acceleration).Rotator();
	DeltaRot.Normalize();

	return FMath::Abs(DeltaRot.Yaw) < 50.f;
}

UGASPCharacterMovementComponent* AGASPCharacter::GetBCharacterMovement() const
{
	return static_cast<UGASPCharacterMovementComponent*>(GetCharacterMovement());
}

void AGASPCharacter::SetOverlayState(const EOverlayState NewOverlayState, const bool bForce)
{
	if (NewOverlayState != OverlayState || bForce)
	{
		OverlayState = NewOverlayState;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OverlayState, this);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetOverlayState(NewOverlayState);
		}
		OverlayStateChanged.Broadcast(NewOverlayState);
	}
}

void AGASPCharacter::SetLocomotionAction(ELocomotionAction NewLocomotionAction, bool bForce)
{
	if (NewLocomotionAction != LocomotionAction || bForce)
	{
		LocomotionAction = NewLocomotionAction;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LocomotionAction, this);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetLocomotionAction(NewLocomotionAction);
		}
		LocomotionActionChanged.Broadcast(NewLocomotionAction);
	}
}

void AGASPCharacter::Server_SetLocomotionAction_Implementation(ELocomotionAction NewLocomotionAction)
{
	SetLocomotionAction(NewLocomotionAction);
}

void AGASPCharacter::Server_SetOverlayState_Implementation(const EOverlayState NewOverlayState)
{
	SetOverlayState(NewOverlayState);
}

void AGASPCharacter::SetReplicatedAcceleration(FVector NewAcceleration)
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

void AGASPCharacter::PlayAudioEvent(const FGameplayTag GameplayTag, const float VolumeMultiplier,
                                    const float PitchMultiplier)
{
	UFoleyAudioBankPrimaryDataAsset* AudioBank = GetFoleyAudioBank();
	if (IsValid(AudioBank))
	{
		USoundBase* Sound{AudioBank->GetSoundFromEvent(GameplayTag)};
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetMesh()->GetComponentLocation(), VolumeMultiplier,
		                                      PitchMultiplier);
	}
}

void AGASPCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	const float VolumeMultiplier = FMath::GetMappedRangeValueClamped<float, float>(
		{-500.f, -900.f}, {.5f, 1.5f}, GetVelocity().Size2D());
	PlayAudioEvent(FoleyJumpTag, VolumeMultiplier);
}

UFoleyAudioBankPrimaryDataAsset* AGASPCharacter::GetFoleyAudioBank()
{
	return FoleyAudioBank;
}

bool AGASPCharacter::CanPlayFootstepSounds()
{
	return MovementMode == ECMovementMode::OnGround || bDoingTraversal;
}

void AGASPCharacter::RefreshGait()
{
	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue,
	                                 FString::Printf(
		                                 TEXT("Desired gait = %s, Gait = %s"),
		                                 *FStructEnumLibrary::GetNameStringByValue(DesiredGait),
		                                 *FStructEnumLibrary::GetNameStringByValue(Gait)));
	if (DesiredGait == EGait::Sprint)
	{
		if (CanSprint())
		{
			SetGait(EGait::Sprint);
			return;
		}
		SetGait(EGait::Run);
		return;
	}

	SetGait(DesiredGait);
}

void AGASPCharacter::OnMovementUpdateSimulatedProxy_Implementation(float DeltaSeconds, FVector OldLocation,
                                                                   FVector OldVelocity)
{
	if (MovementMode != PreviousMovementMode)
	{
		float VolumeMultiplier;
		switch (MovementMode)
		{
		case ECMovementMode::OnGround:
			VolumeMultiplier = FMath::GetMappedRangeValueClamped<float, float>(
				{-500.f, -900.f}, {.5f, 1.5f}, OldVelocity.Z);
			PlayAudioEvent(FoleyJumpTag, VolumeMultiplier);
			break;
		default:
			VolumeMultiplier = FMath::GetMappedRangeValueClamped<float, float>(
				{.0f, 500.f}, {.5f, 1.f}, OldVelocity.Size2D());
			PlayAudioEvent(FoleyLandTag, VolumeMultiplier);
			break;
		}
	}
	PreviousMovementMode = MovementMode;
}
