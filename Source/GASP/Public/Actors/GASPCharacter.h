// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/GASPTraversalComponent.h"
#include "GameFramework/Character.h"
#include "Types/EnumTypes.h"
#include "Types/GameplayTags.h"
#include "Types/StructTypes.h"
#include "GASPCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverlayModeChanged, FGameplayTag, OldOverlayMode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRotationModeChanged, ERotationMode, OldRotationMode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGaitChanged, EGait, OldGait);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementStateChanged, EMovementState, OldMovementState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStanceModeChanged, EStanceMode, OldStanceMode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocomotionActionChanged, FGameplayTag, OldLocomotionAction);

class UGASPCharacterMovementComponent;

UCLASS(Abstract)
class GASP_API AGASPCharacter : public ACharacter
{
	GENERATED_BODY()

	UFUNCTION(BlueprintSetter)
	void SetMovementMode(ECMovementMode NewMovementMode, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetMovementMode(ECMovementMode NewMovementMode);

protected:
	UPROPERTY(BlueprintReadOnly, Transient)
	TObjectPtr<UGASPCharacterMovementComponent> MovementComponent{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UMotionWarpingComponent> MotionWarpingComponent{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Replicated)
	TObjectPtr<class UGASPTraversalComponent> TraversalComponent{};

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Transient)
	EGait DesiredGait{EGait::Run};

	UPROPERTY(BlueprintReadOnly, Transient)
	EGait Gait{EGait::Run};
	UPROPERTY(BlueprintReadOnly, Replicated, Transient)
	ERotationMode RotationMode{ERotationMode::Strafe};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Transient)
	EMovementState MovementState{EMovementState::Idle};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Transient)
	ECMovementMode MovementMode{ECMovementMode::OnGround};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Transient)
	EStanceMode StanceMode{EStanceMode::Stand};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Transient)
	FGameplayTag OverlayMode{OverlayModeTags::Default};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Transient)
	FGameplayTag LocomotionAction{FGameplayTag::EmptyTag};

	UPROPERTY(BlueprintReadOnly, Transient)
	ECMovementMode PreviousMovementMode{ECMovementMode::OnGround};

	UPROPERTY(BlueprintReadOnly, Replicated, Transient)
	FVector_NetQuantize ReplicatedAcceleration{ForceInit};
	UPROPERTY(BlueprintReadOnly, Replicated, Transient)
	FVector_NetQuantize RagdollTargetLocation{ForceInit};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Character", Transient)
	FRagdollingState RagdollingState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> GetUpMontageFront{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> GetUpMontageBack{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 bLimitInitialRagdollSpeed : 1{false};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag FoleyJumpTag{FoleyTags::Jump};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag FoleyLandTag{FoleyTags::Land};

	void SetReplicatedAcceleration(FVector NewAcceleration);

	UFUNCTION(BlueprintPure)
	UAnimMontage* SelectGetUpMontage(bool bRagdollFacingUpward);

	virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
	                                              const FVector& PreviousFloorContactNormal,
	                                              const FVector& PreviousLocation, float TimeDelta) override;

	UFUNCTION(BlueprintNativeEvent)
	void OnMovementUpdateSimulatedProxy(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAudioEvent(const FGameplayTag GameplayTag, const float VolumeMultiplier = 1.f,
	                    const float PitchMultiplier = 1.f);

	virtual void OnJumped_Implementation() override;

	/** Please add a function description */
	UFUNCTION(BlueprintPure, Category = "Traversal")
	FTraversalCheckInputs GetTraversalCheckInputs() const;

public:
	void RefreshGait();

	UFUNCTION(BlueprintCallable, Category="Traversal")
	FTraversalResult TryTraversalAction();
	UFUNCTION(BlueprintPure, Category="Traversal")
	bool IsDoingTraversal() const;
	UPROPERTY(BlueprintAssignable)
	FOnOverlayModeChanged OverlayModeChanged;

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
	void SetMovementState(EMovementState NewMovementState, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetMovementState(EMovementState NewMovementState);

	UFUNCTION(BlueprintCallable)
	void SetStanceMode(EStanceMode NewStanceMode, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetStanceMode(EStanceMode NewStanceMode);

	UFUNCTION(BlueprintSetter)
	void SetOverlayMode(FGameplayTag NewOverlayMode, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetOverlayMode(FGameplayTag NewOverlayMode);

	UFUNCTION(BlueprintSetter)
	void SetLocomotionAction(FGameplayTag NewLocomotionAction, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetLocomotionAction(FGameplayTag NewLocomotionAction);

	UFUNCTION(BlueprintPure)
	virtual bool CanSprint();
	UGASPCharacterMovementComponent* GetBCharacterMovement() const;

	UFUNCTION(BlueprintGetter)
	FORCEINLINE FVector GetReplicatedAcceleration() const
	{
		return ReplicatedAcceleration;
	}

	UFUNCTION(BlueprintGetter)
	FORCEINLINE FGameplayTag GetOverlayMode() const
	{
		return OverlayMode;
	}

	UFUNCTION(BlueprintGetter)
	FORCEINLINE FGameplayTag GetLocomotionAction() const
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
