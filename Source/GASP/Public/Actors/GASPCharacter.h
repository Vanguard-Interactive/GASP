// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types/EnumTypes.h"
#include "GASPCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverlayStateChanged, EOverlayState, NewOverlayState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRotationModeChanged, ERotationMode, NewRotationMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGaitChanged, EGait, NewGait);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementStateChanged, EMovementState, NewMovementState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStanceModeChanged, EStanceMode, NetStanceMode);

class UGASPCharacterMovementComponent;

UCLASS(Abstract)
class GASP_API AGASPCharacter : public ACharacter {
    GENERATED_BODY()

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    virtual void PostInitializeComponents() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode) override;

    UPROPERTY()
    TObjectPtr<UGASPCharacterMovementComponent> MovementComponent {};

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ERotationMode DesiredRotationMode { ERotationMode::Strafe };
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EGait DesiredGait { EGait::Run };

    UPROPERTY(BlueprintReadOnly, Replicated)
    EGait Gait { EGait::Run };
    UPROPERTY(BlueprintReadOnly, Replicated)
    ERotationMode RotationMode { ERotationMode::Strafe };
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
    EMovementState MovementState { EMovementState::Idle };
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
    ECMovementMode MovementMode { ECMovementMode::OnGround };
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
    EStanceMode StanceMode { EStanceMode::Stand };
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
    EOverlayState OverlayState { EOverlayState::Default };

public:
    UPROPERTY(BlueprintAssignable)
    FOnOverlayStateChanged OverlayStateChanged;

    UPROPERTY(BlueprintAssignable)
    FOnGaitChanged GaitChanged;
    UPROPERTY(BlueprintAssignable)
    FOnRotationModeChanged RotationModeChanged;
    UPROPERTY(BlueprintAssignable)
    FOnMovementStateChanged MovementStateChanged;
    UPROPERTY(BlueprintAssignable)
    FOnStanceModeChanged StanceModeChanged;

    // Sets default values for this character's properties
    explicit AGASPCharacter(const FObjectInitializer& ObjectInitializer);
    AGASPCharacter() = default;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    virtual void PostRegisterAllComponents() override;

    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

    /****************************
     *		Input actions		*
     ****************************/
    UFUNCTION(BlueprintCallable)
    void MoveAction(const FVector2D& Value);
    UFUNCTION(BlueprintCallable)
    void LookAction(const FVector2D& Value);

    /****************************
     *		Movement States		*
     ****************************/
    UFUNCTION(BlueprintSetter)
    void SetGait(EGait NewGait, bool bForce = false);
    UFUNCTION(Server, Reliable)
    void Server_SetGait(EGait NewGait);

    UFUNCTION(BlueprintSetter)
    void SetRotationMode(ERotationMode NewRotationMode, bool bForce = false);
    UFUNCTION(Server, Reliable)
    void Server_SetRotationMode(ERotationMode NewRotationMode);

    UFUNCTION(BlueprintSetter)
    void SetMovementMode(ECMovementMode NewMovementMode, bool bForce = false);
    UFUNCTION(Server, Reliable)
    void Server_SetMovementMode(ECMovementMode NewMovementMode);

    UFUNCTION(BlueprintSetter)
    void SetMovementState(EMovementState NewMovementState, bool bForce = false);
    UFUNCTION(Server, Reliable)
    void Server_SetMovementState(EMovementState NewMovementState);

    UFUNCTION(BlueprintCallable)
    void SetStanceMode(EStanceMode NewStanceMode, bool bForce = false);
    UFUNCTION(Server, Reliable)
    void Server_SetStanceMode(EStanceMode NewStanceMode);

    UFUNCTION(BlueprintSetter)
    void SetOverlayState(EOverlayState NewOverlayState, bool bForce = false);
    UFUNCTION(Server, Reliable)
    void Server_SetOverlayState(EOverlayState NewOverlayState);

    UFUNCTION(BlueprintNativeEvent)
    void PlayMontage(UAnimMontage* MontageToPlay, float InPlayRate = 1.f,
        EMontagePlayReturnType ReturnValueType = EMontagePlayReturnType::MontageLength,
        float InTimeToStartMontageAt = 0.f, bool bStopAllMontages = true);
    UFUNCTION(Server, Reliable)
    void Server_PlayMontage(UAnimMontage* MontageToPlay, float InPlayRate = 1.f,
        EMontagePlayReturnType ReturnValueType = EMontagePlayReturnType::MontageLength,
        float InTimeToStartMontageAt = 0.f, bool bStopAllMontages = true);
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayMontage(UAnimMontage* MontageToPlay, float InPlayRate = 1.f,
        EMontagePlayReturnType ReturnValueType = EMontagePlayReturnType::MontageLength,
        float InTimeToStartMontageAt = 0.f, bool bStopAllMontages = true);

    UFUNCTION(BlueprintPure)
    virtual bool CanSprint();
    UGASPCharacterMovementComponent* GetBCharacterMovement() const;

    UFUNCTION(BlueprintGetter)
    FORCEINLINE EOverlayState GetOverlayState() const
    {
        return OverlayState;
    }
    UFUNCTION(BlueprintGetter)
    FORCEINLINE EGait GetGait() const
    {
        return Gait;
    }
    UFUNCTION(BlueprintGetter)
    FORCEINLINE ERotationMode GetRotationMode() const
    {
        return RotationMode;
    }
    UFUNCTION(BlueprintGetter)
    FORCEINLINE ECMovementMode GetMovementMode() const
    {
        return MovementMode;
    }
    UFUNCTION(BlueprintGetter)
    FORCEINLINE EMovementState GetMovementState() const
    {
        return MovementState;
    }

    UFUNCTION(BlueprintGetter)
    FORCEINLINE EStanceMode GetStanceMode() const
    {
        return StanceMode;
    }
};
