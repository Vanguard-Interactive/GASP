// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/GASPTraversalComponent.h"
#include "GameFramework/Character.h"
#include "Types/EnumTypes.h"
#include "Types/TagTypes.h"
#include "Types/StructTypes.h"
#include "GASPCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverlayModeChanged, FGameplayTag, OldOverlayMode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRotationModeChanged, ERotationMode, OldRotationMode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGaitChanged, EGait, OldGait);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementStateChanged, EMovementState, OldMovementState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStanceModeChanged, FGameplayTag, OldStanceMode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocomotionActionChanged, FGameplayTag, OldLocomotionAction);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementModeChanged, FGameplayTag, OldMovementMode);

class UGASPCharacterMovementComponent;

UCLASS()
class GASP_API AGASPCharacter : public ACharacter
{
	GENERATED_BODY()

	FTimerHandle LandedHandle;


	UFUNCTION(BlueprintSetter)
	void SetMovementMode(const FGameplayTag NewMovementMode, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetMovementMode(const FGameplayTag NewMovementMode);

protected:
	UPROPERTY(EditAnywhere, Category="PoseSearchData|Choosers", BlueprintReadOnly)
	TObjectPtr<UChooserTable> OverlayTable{nullptr};

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_RotationMode, Transient)
	ERotationMode RotationMode{ERotationMode::Strafe};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_MovementState, Transient)
	EMovementState MovementState{EMovementState::Idle};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_MovementMode, Transient)
	FGameplayTag MovementMode{MovementModeTags::Grounded};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_StanceMode, Transient)
	FGameplayTag StanceMode{StanceTags::Standing};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_OverlayMode, Transient)
	FGameplayTag OverlayMode{OverlayModeTags::Default};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_LocomotionAction, Transient)
	FGameplayTag LocomotionAction{FGameplayTag::EmptyTag};

	UPROPERTY(BlueprintReadOnly, Transient)
	FGameplayTag PreviousMovementMode{MovementModeTags::Grounded};

	UPROPERTY(BlueprintReadOnly, Replicated, Transient)
	FVector_NetQuantize ReplicatedAcceleration{ForceInit};
	UPROPERTY(BlueprintReadOnly, Replicated, Transient)
	FVector_NetQuantize RagdollTargetLocation{ForceInit};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Character", Transient)
	FRagdollingState RagdollingState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Character")
	TObjectPtr<UAnimMontage> GetUpMontageFront{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Character")
	TObjectPtr<UAnimMontage> GetUpMontageBack{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Character")
	uint8 bLimitInitialRagdollSpeed : 1{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Character")
	FGameplayTag FoleyJumpTag{FoleyTags::Jump};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Character")
	FGameplayTag FoleyLandTag{FoleyTags::Land};

	void SetReplicatedAcceleration(const FVector& NewAcceleration);

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

	virtual void Landed(const FHitResult& Hit) override;

	/** Please add a function description */
	UFUNCTION(BlueprintPure, Category = "Traversal")
	FTraversalCheckInputs GetTraversalCheckInputs() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta=(ClampMin="0.0", ClampMax="1.0"))
	float AnalogMovementThreshold{.7f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	EAnalogStickBehaviorMode MovementStickMode{EAnalogStickBehaviorMode::FixedSingleGait};

	UFUNCTION(BlueprintPure, Category = "Input")
	bool HasFullMovementInput() const;

	UFUNCTION(BlueprintPure, Category = "Input")
	FVector2D GetMovementInputScaleValue(const FVector2D InVector) const;

public:
	void RefreshGait();

	UFUNCTION(BlueprintCallable, Category="Traversal")
	FTraversalResult TryTraversalAction() const;
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
	UPROPERTY(BlueprintAssignable)
	FOnMovementModeChanged MovementModeChanged;

	UFUNCTION(BlueprintNativeEvent)
	void OnOverlayModeChanged(const FGameplayTag OldOverlayMode);

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
	UFUNCTION(BlueprintCallable)
	void SetGait(const EGait NewGait, bool bForce = false);

	UFUNCTION(BlueprintCallable)
	void SetDesiredGait(const EGait NewGait, bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetDesiredGait(const EGait NewGait);

	UFUNCTION(BlueprintCallable)
	void SetRotationMode(const ERotationMode NewRotationMode, const bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetRotationMode(const ERotationMode NewRotationMode);

	UFUNCTION(BlueprintCallable)
	void SetMovementState(const EMovementState NewMovementState, const bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetMovementState(const EMovementState NewMovementState);

	UFUNCTION(BlueprintCallable)
	void SetStanceMode(const FGameplayTag NewStanceMode, const bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetStanceMode(const FGameplayTag NewStanceMode);

	UFUNCTION(BlueprintCallable)
	void SetOverlayMode(const FGameplayTag NewOverlayMode, const bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetOverlayMode(const FGameplayTag NewOverlayMode);


	UFUNCTION(BlueprintCallable)
	void SetLocomotionAction(const FGameplayTag NewLocomotionAction, const bool bForce = false);
	UFUNCTION(Server, Reliable)
	void Server_SetLocomotionAction(const FGameplayTag NewLocomotionAction);

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
	FORCEINLINE FGameplayTag GetMovementMode() const
	{
		return MovementMode;
	}

	UFUNCTION(BlueprintGetter)
	FORCEINLINE EMovementState GetMovementState() const
	{
		return MovementState;
	}

	UFUNCTION(BlueprintGetter)
	FORCEINLINE FGameplayTag GetStanceMode() const
	{
		return StanceMode;
	}

	// Ragdolling
	bool IsRagdollingAllowedToStart() const;

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

	UFUNCTION()
	void OnRep_OverlayMode(const FGameplayTag& OldOverlayMode);
	UFUNCTION()
	void OnRep_Gait(const EGait& OldGait);
	UFUNCTION()
	void OnRep_StanceMode(const FGameplayTag& OldStanceMode);
	UFUNCTION()
	void OnRep_MovementMode(const FGameplayTag& OldMovementMode);
	UFUNCTION()
	void OnRep_RotationMode(const ERotationMode& OldRotationMode);
	UFUNCTION()
	void OnRep_MovementState(const EMovementState& OldMovementState);
	UFUNCTION()
	void OnRep_LocomotionAction(const FGameplayTag& OldLocomotionAction);

public:
	bool IsRagdollingAllowedToStop() const;

	UFUNCTION(BlueprintCallable, Category = "GASP|Character", Meta = (ReturnDisplayName = "Success"))
	bool StopRagdolling();

	UFUNCTION(BlueprintImplementableEvent)
	void OnStartRagdolling();
	UFUNCTION(BlueprintImplementableEvent)
	void OnStopRagdolling();

	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer StateContainer;

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
