// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/GASPCharacterMovementComponent.h"
#include "GameFramework/Character.h"

UGASPCharacterMovementComponent::UGASPCharacterMovementComponent()
{
	SetNetworkMoveDataContainer(MoveDataContainer);

	bRunPhysicsWithNoController = true;
	bAllowPhysicsRotationDuringAnimRootMotion = true;
	bNetworkAlwaysReplicateTransformUpdateTimestamp = true;
	
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

void UGASPCharacterMovementComponent::FGASPCharacterNetworkMoveData::ClientFillNetworkMoveData(
	const FSavedMove_Character& Move, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(Move, MoveType);

	const FGASPSavedMove& SavedMove{static_cast<const FGASPSavedMove&>(Move)};

	SavedRotationMode = SavedMove.SavedRotationMode;
	SavedGait = SavedMove.SavedGait;
}

UGASPCharacterMovementComponent::FGASPCharacterNetworkMoveDataContainer::FGASPCharacterNetworkMoveDataContainer()
{
	NewMoveData = &MoveData[0];
	PendingMoveData = &MoveData[1];
	OldMoveData = &MoveData[2];
}

bool UGASPCharacterMovementComponent::FGASPSavedMove::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FGASPSavedMove* NewCombineMove = StaticCast<FGASPSavedMove*>(NewMove.Get());
	
	return SavedRotationMode == NewCombineMove->SavedRotationMode && SavedGait == NewCombineMove->SavedGait &&
		Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UGASPCharacterMovementComponent::FGASPSavedMove::CombineWith(const FSavedMove_Character* OldMove,
	ACharacter* InCharacter, APlayerController* PC, const FVector& OldStartLocation)
{
	const auto OriginalRotation{OldMove->StartRotation};
	const auto OriginalRelativeRotation{OldMove->StartAttachRelativeRotation};
	
	const auto* NewUpdatedComponent{InCharacter->GetCharacterMovement()->UpdatedComponent.Get()};
	
	auto* MutablePreviousMove{const_cast<FSavedMove_Character*>(OldMove)};
	
	MutablePreviousMove->StartRotation = NewUpdatedComponent->GetComponentRotation();
	MutablePreviousMove->StartAttachRelativeRotation = NewUpdatedComponent->GetRelativeRotation();
	
	Super::CombineWith(OldMove, InCharacter, PC, OldStartLocation);
	
	MutablePreviousMove->StartRotation = OriginalRotation;
	MutablePreviousMove->StartAttachRelativeRotation = OriginalRelativeRotation;
	}

void UGASPCharacterMovementComponent::FGASPSavedMove::Clear()
{
	Super::Clear();

	SavedGait = EGait::Run;
	bSavedRotationModeUpdate = false;
	SavedRotationMode = ERotationMode::OrientToMovement;
}

uint8 UGASPCharacterMovementComponent::FGASPSavedMove::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedRotationModeUpdate)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

void UGASPCharacterMovementComponent::FGASPSavedMove::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
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

void UGASPCharacterMovementComponent::FGASPSavedMove::PrepMoveFor(ACharacter* C)
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
	return MakeShared<FGASPSavedMove>();
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

void UGASPCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	Super::PhysicsRotation(DeltaTime);

	if (IsMovingOnGround())
	{
		// UE_LOG(LogTemp, Display, TEXT(""))
		MaxWalkSpeed = GaitSettings.GetMappedSpeed(SafeGait, Velocity, GetLastUpdateRotation());
	}
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
			bSafeRotationModeUpdate = false;
		}
	}
}

void UGASPCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (IsMovingOnGround() || IsFalling())
	{
		RotationRate = FRotator(0.f,  IsFalling() ? InAirRotationYaw : -1.f, 0.f);
	}
}

float UGASPCharacterMovementComponent::CalculateGroundFriction()
{
	if (SafeGait == EGait::Run || SafeGait == EGait::Walk) return MovementInformation.GroundFriction;
	
	return FMath::GetMappedRangeValueClamped<float, float>({ 0.f, StaticCast<float>(GaitSettings.GetSpeedForGait(EGait::Run).X) },
		{ MovementInformation.GroundFriction, 3.f }, Velocity.Size2D());
}

void UGASPCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	GroundFriction = CalculateGroundFriction();
	
	Super::PhysWalking(deltaTime, Iterations);
}

bool UGASPCharacterMovementComponent::HasMovementInputVector() const
{
	return !GetPendingInputVector().IsZero();
}

void UGASPCharacterMovementComponent::UpdateRotationMode()
{
	switch (SafeRotationMode)
	{
	case ERotationMode::OrientToMovement:
		bUseControllerDesiredRotation = false;
		bOrientRotationToMovement = true;
		break;
	default:
		bUseControllerDesiredRotation = true;
		bOrientRotationToMovement = false;
		break;
	}
}

void UGASPCharacterMovementComponent::Server_SetGait_Implementation(const EGait NewGait)
{
	SafeGait = NewGait;
}

void UGASPCharacterMovementComponent::SetGait(const EGait NewGait)
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

void UGASPCharacterMovementComponent::SetRotationMode(const ERotationMode NewRotationMode)
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

void UGASPCharacterMovementComponent::Server_SetRotationMode_Implementation(const ERotationMode NewRotationMode)
{
	SafeRotationMode = NewRotationMode;
}

float UGASPCharacterMovementComponent::GetMaxAcceleration() const
{
	return FMath::GetMappedRangeValueClamped<float, float>({ 300.f, StaticCast<float>(
		GaitSettings.GetSpeedForGait(EGait::Sprint).X) }, { MovementInformation.MaxAcceleration,
			300.f }, Velocity.Size2D());
}

float UGASPCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (IsMovingOnGround())
	{
		return HasMovementInputVector() ? MovementInformation.MovingMaxBrakingDeceleration :
        		MovementInformation.NotMovingMaxBrakingDeceleration;
	}
	return Super::GetMaxBrakingDeceleration();
}
