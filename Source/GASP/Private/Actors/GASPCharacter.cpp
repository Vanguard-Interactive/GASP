// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/GASPCharacter.h"
#include "Components/GASPCharacterMovementComponent.h"
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

    // bAlwaysRelevant = true;
    bReplicates = true;
    SetReplicatingMovement(true);

    GetMesh()->bEnableUpdateRateOptimizations = false;

    MovementComponent = Cast<UGASPCharacterMovementComponent>(GetCharacterMovement());
}

// Called when the game starts or when spawned
void AGASPCharacter::BeginPlay()
{
    Super::BeginPlay();

    SetGait(DesiredGait, true);
    SetRotationMode(DesiredRotationMode, true);
    SetOverlayState(OverlayState, true);
}

// Called every frame
void AGASPCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // if (!IsValid(MovementComponent))
    // return;
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
    Parameters.Condition = COND_SkipOwner;

    Parameters.Condition = COND_None;
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Gait, Parameters);
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, RotationMode, Parameters);
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MovementMode, Parameters);
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MovementState, Parameters);
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, StanceMode, Parameters);

    Parameters.Condition = COND_None;
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OverlayState, Parameters);
}

void AGASPCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PrevCustomMode);

    if (!ensure(MovementComponent))
        return;

    if (MovementComponent->IsMovingOnGround()) {
        SetMovementMode(ECMovementMode::OnGround);
    } else if (MovementComponent->IsFalling()) {
        SetMovementMode(ECMovementMode::InAir);
    }
}

void AGASPCharacter::SetGait(const EGait NewGait, const bool bForce)
{
    if (!ensure(MovementComponent))
        return;

    if (NewGait != Gait || bForce) {

        Gait = NewGait;
        MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Gait, this);

        MovementComponent->SetGait(NewGait);

        if (GetLocalRole() == ROLE_AutonomousProxy) {
            Server_SetGait(NewGait);
        }

        GaitChanged.Broadcast(NewGait);
    }
}

void AGASPCharacter::SetRotationMode(const ERotationMode NewRotationMode, const bool bForce)
{
    if (!ensure(MovementComponent))
        return;

    if (NewRotationMode != RotationMode || bForce) {

        RotationMode = NewRotationMode;
        MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, RotationMode, this);

        MovementComponent->SetRotationMode(NewRotationMode);

        if (GetLocalRole() == ROLE_AutonomousProxy) {
            Server_SetRotationMode(NewRotationMode);
        }

        RotationModeChanged.Broadcast(NewRotationMode);
    }
}

void AGASPCharacter::SetMovementMode(const ECMovementMode NewMovementMode, const bool bForce)
{
    if (!ensure(MovementComponent))
        return;

    if (NewMovementMode != MovementMode || bForce) {
        MovementMode = NewMovementMode;
        MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MovementMode, this);
        if (GetLocalRole() == ROLE_AutonomousProxy) {
            Server_SetMovementMode(NewMovementMode);
        }
    }
}

void AGASPCharacter::Server_SetMovementMode_Implementation(const ECMovementMode NewMovementMode)
{
    MovementMode = NewMovementMode;
}

void AGASPCharacter::Server_SetRotationMode_Implementation(const ERotationMode NewRotationMode)
{
    SetRotationMode(NewRotationMode);
}

void AGASPCharacter::Server_SetGait_Implementation(const EGait NewGait)
{
    SetGait(NewGait);
}

void AGASPCharacter::SetMovementState(const EMovementState NewMovementState, const bool bForce)
{
    if (!ensure(MovementComponent))
        return;

    if (NewMovementState != MovementState || bForce) {

        MovementState = NewMovementState;
        MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MovementState, this);

        if (GetLocalRole() == ROLE_AutonomousProxy) {
            Server_SetMovementState(NewMovementState);
        }
        MovementStateChanged.Broadcast(NewMovementState);
    }
}

void AGASPCharacter::SetStanceMode(EStanceMode NewStanceMode, bool bForce)
{
    if (StanceMode != NewStanceMode || bForce) {
        StanceMode = NewStanceMode;
        MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, StanceMode, this);

        UE_LOG(LogTemp, Warning, TEXT("Crouch"))

        if (GetLocalRole() == ROLE_AutonomousProxy) {
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
    MovementState = NewMovementState;
}

void AGASPCharacter::MoveAction(const FVector2D& Value)
{
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

void AGASPCharacter::PlayMontage_Implementation(UAnimMontage* MontageToPlay, float InPlayRate,
    EMontagePlayReturnType ReturnValueType, float InTimeToStartMontageAt,
    bool bStopAllMontages)
{
    if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance()) {
        AnimInst->Montage_Play(MontageToPlay, InPlayRate, ReturnValueType, InTimeToStartMontageAt, bStopAllMontages);
    }

    Server_PlayMontage(MontageToPlay, InPlayRate, ReturnValueType, InTimeToStartMontageAt, bStopAllMontages);
}

void AGASPCharacter::Server_PlayMontage_Implementation(UAnimMontage* MontageToPlay, float InPlayRate,
    const EMontagePlayReturnType ReturnValueType,
    const float InTimeToStartMontageAt, const bool bStopAllMontages)
{
    if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance()) {
        AnimInst->Montage_Play(MontageToPlay, InPlayRate, ReturnValueType, InTimeToStartMontageAt, bStopAllMontages);
    }

    ForceNetUpdate();
    Multicast_PlayMontage(MontageToPlay, InPlayRate, ReturnValueType, InTimeToStartMontageAt, bStopAllMontages);
}

void AGASPCharacter::Multicast_PlayMontage_Implementation(UAnimMontage* MontageToPlay, float InPlayRate,
    EMontagePlayReturnType ReturnValueType,
    float InTimeToStartMontageAt, bool bStopAllMontages)
{
    if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance()) {
        AnimInst->Montage_Play(MontageToPlay, InPlayRate, ReturnValueType, InTimeToStartMontageAt, bStopAllMontages);
    }
}

bool AGASPCharacter::CanSprint()
{
    if (RotationMode == ERotationMode::OrientToMovement)
        return true;

    const FVector Acceleration { IsLocallyControlled() ? GetPendingMovementInputVector()
                                                       : MovementComponent->GetCurrentAcceleration() };

    return UAnimationUtils::CalculateDirection(Acceleration, GetActorRotation()) < 50.f;
    //const FRotator Rotation { GetActorRotation() - FRotationMatrix::MakeFromX(Acceleration).Rotator() };
    //return FMath::Abs(Rotation.Yaw) < 50.f;
}

UGASPCharacterMovementComponent* AGASPCharacter::GetBCharacterMovement() const
{
    return StaticCast<UGASPCharacterMovementComponent*>(GetCharacterMovement());
}

void AGASPCharacter::SetOverlayState(const EOverlayState NewOverlayState, const bool bForce)
{
    if (NewOverlayState != OverlayState || bForce) {
        OverlayState = NewOverlayState;
        MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OverlayState, this);

        if (GetLocalRole() == ROLE_AutonomousProxy) {
            Server_SetOverlayState(NewOverlayState);
        }
        OverlayStateChanged.Broadcast(NewOverlayState);
    }
}

void AGASPCharacter::Server_SetOverlayState_Implementation(const EOverlayState NewOverlayState)
{
    OverlayState = NewOverlayState;
}
