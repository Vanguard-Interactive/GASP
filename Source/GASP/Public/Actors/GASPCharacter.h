// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "Interfaces/FoleyAudioBankInterface.h"
#include "Types/EnumTypes.h"
#include "Types/StructTypes.h"
#include "GASPCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverlayStateChanged, EOverlayState, NewOverlayState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRotationModeChanged, ERotationMode, NewRotationMode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGaitChanged, EGait, NewGait);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementStateChanged, EMovementState, NewMovementState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStanceModeChanged, EStanceMode, NetStanceMode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocomotionActionChanged, ELocomotionAction, NewLocomotionAction);

class UGASPCharacterMovementComponent;

UCLASS(Abstract)
class GASP_API AGASPCharacter : public ACharacter, public IFoleyAudioBankInterface
{
	GENERATED_BODY()

protected:
	UFUNCTION()
	void OnSoundsPreloaded();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode) override;
	UPROPERTY()
	TObjectPtr<UGASPCharacterMovementComponent> MovementComponent{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERotationMode DesiredRotationMode{ERotationMode::Strafe};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	EGait DesiredGait{EGait::Run};

	UPROPERTY(BlueprintReadOnly)
	EGait Gait{EGait::Run};
	UPROPERTY(BlueprintReadOnly, Replicated)
	ERotationMode RotationMode{ERotationMode::Strafe};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Transient)
	EMovementState MovementState{EMovementState::Idle};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Transient)
	ECMovementMode MovementMode{ECMovementMode::OnGround};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Transient)
	EStanceMode StanceMode{EStanceMode::Stand};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Transient)
	EOverlayState OverlayState{EOverlayState::Default};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Transient)
	ELocomotionAction LocomotionAction{ELocomotionAction::None};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	ECMovementMode PreviousMovementMode{ECMovementMode::OnGround};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Transient)
	FVector_NetQuantize ReplicatedAcceleration{ForceInit};
	UPROPERTY(BlueprintReadOnly, Replicated)
	FVector_NetQuantize RagdollTargetLocation{ForceInit};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Character", Transient)
	FRagdollingState RagdollingState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	TObjectPtr<UFoleyAudioBankPrimaryDataAsset> FoleyAudioBank;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> GetUpMontageFront{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> GetUpMontageBack{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bLimitInitialRagdollSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag FoleyJumpTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag FoleyLandTag;

	void SetReplicatedAcceleration(FVector NewAcceleration);

	UFUNCTION(BlueprintPure)
	UAnimMontage* SelectGetUpMontage(bool bRagdollFacingUpward);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDoingTraversal{false};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float IgnoreCorrectionsDelay{.2f};

	virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
	                                              const FVector& PreviousFloorContactNormal,
	                                              const FVector& PreviousLocation, float TimeDelta) override;

	UFUNCTION(BlueprintNativeEvent)
	void OnMovementUpdateSimulatedProxy(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	UFUNCTION(BlueprintCallable)
	void PlayAudioEvent(const FGameplayTag GameplayTag, const float VolumeMultiplier = 1.f,
	                    const float PitchMultiplier = 1.f);

	virtual void OnJumped_Implementation() override;

public:
	virtual UFoleyAudioBankPrimaryDataAsset* GetFoleyAudioBank() override;
	virtual bool CanPlayFootstepSounds() override;

	void RefreshGait();

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
	UPROPERTY(BlueprintAssignable)
	FOnLocomotionActionChanged LocomotionActionChanged;

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

	UFUNCTION(BlueprintSetter)
	void SetDesiredGait(EGait NewGait, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetDesiredGait(EGait NewGait);

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

	UFUNCTION(BlueprintSetter)
	void SetLocomotionAction(ELocomotionAction NewLocomotionAction, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetLocomotionAction(ELocomotionAction NewLocomotionAction);

	UFUNCTION(BlueprintPure)
	virtual bool CanSprint();
	UGASPCharacterMovementComponent* GetBCharacterMovement() const;

	UFUNCTION(BlueprintGetter)
	FORCEINLINE FVector GetReplicatedAcceleration() const
	{
		return ReplicatedAcceleration;
	}

	UFUNCTION(BlueprintGetter)
	FORCEINLINE EOverlayState GetOverlayState() const
	{
		return OverlayState;
	}

	UFUNCTION(BlueprintGetter)
	FORCEINLINE ELocomotionAction GetLocomotionAction() const
	{
		return LocomotionAction;
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

	bool IsRagdollingAllowedToStart() const;

	// Ragdolling

	const FRagdollingState& GetRagdollingState() const
	{
		return RagdollingState;
	}

public:
	UFUNCTION(BlueprintCallable, Category = "GASP|Character")
	void StartRagdolling();

private:
	UFUNCTION(Server, Reliable)
	void ServerStartRagdolling();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartRagdolling();

	void StartRagdollingImplementation();

public:
	bool IsRagdollingAllowedToStop() const;

	UFUNCTION(BlueprintCallable, Category = "GASP|Character", Meta = (ReturnDisplayName = "Success"))
	bool StopRagdolling();

private:
	UFUNCTION(Server, Reliable)
	void ServerStopRagdolling();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopRagdolling();

	void StopRagdollingImplementation();

	void SetRagdollTargetLocation(const FVector& NewTargetLocation);

	UFUNCTION(Server, Unreliable)
	void ServerSetRagdollTargetLocation(const FVector_NetQuantize& NewTargetLocation);

	void RefreshRagdolling(float DeltaTime);

	FVector RagdollTraceGround(bool& bGrounded) const;

	void ConstraintRagdollSpeed() const;
};
