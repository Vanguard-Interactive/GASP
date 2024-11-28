// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/GASPCharacterMovementComponent.h"
#include "GameFramework/Character.h"

UGASPCharacterMovementComponent::UGASPCharacterMovementComponent()
{
	RotationRate = FRotator(0.f, -1.f, 0.f);
	BrakingDecelerationWalking = 1500.f;
	MinAnalogWalkSpeed = 150.f;
	MaxWalkSpeed = 500.f;
	GroundFriction = 5.f;
	BrakingFrictionFactor = 1.f;
	BrakingFriction = 0.f;
	MaxAcceleration = 800.f;
	PerchRadiusThreshold = 20.0f;
	bUseFlatBaseForFloorChecks = true;
	bUseSeparateBrakingFriction = true;
}

bool UGASPCharacterMovementComponent::FSavedMove_Base::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_Base* NewCombineMove = StaticCast<FSavedMove_Base*>(NewMove.Get());

	if (bSavedRotationModeUpdate != NewCombineMove->bSavedRotationModeUpdate)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UGASPCharacterMovementComponent::FSavedMove_Base::Clear()
{
	Super::Clear();

	SavedGait = EGait::Run;
	bSavedRotationModeUpdate = false;
	SavedRotationMode = ERotationMode::OrientToMovement;
}

uint8 UGASPCharacterMovementComponent::FSavedMove_Base::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedRotationModeUpdate)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

void UGASPCharacterMovementComponent::FSavedMove_Base::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const UGASPCharacterMovementComponent* CharacterMovement = Cast<UGASPCharacterMovementComponent>(C->GetCharacterMovement());

	if (CharacterMovement)
	{
		SavedGait = CharacterMovement->SafeGait;
		bSavedRotationModeUpdate = CharacterMovement->bSafeRotationModeUpdate;
		SavedRotationMode = CharacterMovement->SafeRotationMode;
	}
}

void UGASPCharacterMovementComponent::FSavedMove_Base::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	UGASPCharacterMovementComponent* CharacterMovement = Cast<UGASPCharacterMovementComponent>(C->GetCharacterMovement());

	CharacterMovement->SafeGait = SavedGait;
	CharacterMovement->SafeRotationMode = SavedRotationMode;
}

UGASPCharacterMovementComponent::FNetworkPredictionData_Client_Base::FNetworkPredictionData_Client_Base(
	const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UGASPCharacterMovementComponent::FNetworkPredictionData_Client_Base::AllocateNewMove()
{
	return MakeShared<FSavedMove_Base>();
}

FNetworkPredictionData_Client* UGASPCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (!ClientPredictionData)
	{
		UGASPCharacterMovementComponent* MutableThis = const_cast<UGASPCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Base(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UGASPCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bSafeRotationModeUpdate = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}


void UGASPCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (!CharacterOwner) return;

	if (IsMovingOnGround())
	{
		if (bSafeRotationModeUpdate)
		{
			UpdateRotationMode();
		}
		
		MaxWalkSpeed = GaitSettings.GetMappedSpeed(SafeGait, Velocity, GetActorTransform());
	}
}

void UGASPCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	RotationRate = FRotator(0.f,  IsFalling() ? 200.f : -1.f, 0.f);
}

float UGASPCharacterMovementComponent::CalculateMaxAcceleration()
{
	if (SafeGait == EGait::Run || SafeGait == EGait::Walk) return MovementInformation.MaxAcceleration;
	
	return FMath::GetMappedRangeValueClamped<float, float>({ 300.f, StaticCast<float>(GaitSettings.GetSpeedForGait(EGait::Sprint).X) },
		{ MovementInformation.MaxAcceleration, 300.f }, Velocity.Size2D());
}

float UGASPCharacterMovementComponent::CalculateGroundFriction()
{
	if (SafeGait == EGait::Run || SafeGait == EGait::Walk) return MovementInformation.GroundFriction;
	
	return FMath::GetMappedRangeValueClamped<float, float>({ 0.f, StaticCast<float>(GaitSettings.GetSpeedForGait(EGait::Run).X) },
		{ MovementInformation.GroundFriction, 3.f }, Velocity.Size2D());
}

float UGASPCharacterMovementComponent::CalculateBrakingDecelerationWalking()
{
	return HasMovementInputVector() ? MovementInformation.MovingMaxBrakingDeceleration :
		MovementInformation.NotMovingMaxBrakingDeceleration;
}

void UGASPCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	MaxAcceleration = CalculateMaxAcceleration();
	BrakingDecelerationWalking = CalculateBrakingDecelerationWalking();
	
	GroundFriction = CalculateGroundFriction();
	
	Super::PhysWalking(deltaTime, Iterations);
}

bool UGASPCharacterMovementComponent::HasMovementInputVector()
{
	return GetPendingInputVector() != FVector::ZeroVector;
}

void UGASPCharacterMovementComponent::UpdateRotationMode()
{
	switch (SafeRotationMode)
	{
	case ERotationMode::Strafe:
		bUseControllerDesiredRotation = true;
		bOrientRotationToMovement = false;
		break;
	default:
		bUseControllerDesiredRotation = false;
		bOrientRotationToMovement = true;
		break;
	}
}

void UGASPCharacterMovementComponent::Server_SetGait_Implementation(EGait NewGait)
{
	SafeGait = NewGait;
}

void UGASPCharacterMovementComponent::SetGait(EGait NewGait)
{
	if (NewGait != SafeGait)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			SafeGait = NewGait;
			if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
			{
				Server_SetGait(NewGait);
			}
		}
	}
}

void UGASPCharacterMovementComponent::SetRotationMode(ERotationMode NewRotationMode)
{
	if (NewRotationMode != SafeRotationMode)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			SafeRotationMode = NewRotationMode;
			if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
			{
				Server_SetRotationMode(NewRotationMode);
			}
			bSafeRotationModeUpdate = true;
			return;
		}
		if (!PawnOwner->HasAuthority())
		{
			UpdateRotationMode();
		}
	}
}

void UGASPCharacterMovementComponent::Server_SetRotationMode_Implementation(ERotationMode NewRotationMode)
{
	SafeRotationMode = NewRotationMode;
}
