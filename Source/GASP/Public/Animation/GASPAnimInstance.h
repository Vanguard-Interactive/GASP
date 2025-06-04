// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/GASPAnimInstanceProxy.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimNodeReference.h"
#include "BoneControllers/AnimNode_FootPlacement.h"
#include "BoneControllers/AnimNode_OffsetRootBone.h"
#include "BoneControllers/AnimNode_OrientationWarping.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "Types/StructTypes.h"
#include "GASPAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class GASP_API UGASPAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	FTimerHandle LandedHandle;

protected:
	/**************
	 * References *
	 *************/
	UPROPERTY(BlueprintReadOnly, Category = "References", Transient)
	TWeakObjectPtr<class AGASPCharacter> CachedCharacter{};

	UPROPERTY(BlueprintReadOnly, Category = "References", Transient)
	TWeakObjectPtr<class UGASPCharacterMovementComponent> CachedMovement{};

	/******************
	 * Character state *
	 ******************/
	UPROPERTY(EditAnywhere, Category = "CharacterInformation|General", BlueprintReadOnly, Transient)
	FAnimUtilityNames AnimNames{};

	UPROPERTY(VisibleAnywhere, Category = "CharacterInformation|General", BlueprintReadOnly, Transient)
	EMovementDirection MovementDirection{EMovementDirection::F};
	UPROPERTY(VisibleAnywhere, Category = "LocomotionAction", BlueprintReadOnly, Transient)
	FGameplayTag LocomotionAction{FGameplayTag::EmptyTag};

	UPROPERTY(VisibleAnywhere, Category = "CharacterInfromation|States", BlueprintReadOnly, Transient)
	EGait Gait{};
	UPROPERTY(VisibleAnywhere, Category = "CharacterInfromation|States", BlueprintReadOnly, Transient)
	EMovementState MovementState{};
	UPROPERTY(VisibleAnywhere, Category = "CharacterInfromation|States", BlueprintReadOnly, Transient)
	ERotationMode RotationMode{};
	UPROPERTY(VisibleAnywhere, Category = "CharacterInfromation|States", BlueprintReadOnly, Transient)
	FGameplayTag MovementMode{};
	UPROPERTY(VisibleAnywhere, Category = "CharacterInfromation|States", BlueprintReadOnly, Transient)
	FGameplayTag StanceMode{};
	UPROPERTY(VisibleAnywhere, Category = "CharacterInfromation|States", BlueprintReadOnly, Transient)
	FGameplayTagContainer StateContainer{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|General", Transient)
	FCharacterInfo CharacterInfo{};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|General", Transient)
	FGASPBlendStackInputs BlendStackInputs{};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionMatching", Transient)
	FMotionMatchingInfo BlendStack;
	UPROPERTY(VisibleAnywhere, Category = "LocomotionAction|Information", BlueprintReadOnly, Transient)
	FRagdollingAnimationState RagdollingState;
	UPROPERTY(VisibleAnywhere, Category = "Additive|Poses", BlueprintReadOnly, Transient)
	FGASPBlendPoses BlendPoses;
	UPROPERTY(VisibleAnywhere, Category = "Additive|Poses", BlueprintReadOnly, Transient)
	FBlendStackMachine BlendStackMachine;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	FGameplayTag PreviousStanceMode{};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	EGait PreviousGait{EGait::Run};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	EMovementState PreviousMovementState{EMovementState::Idle};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	ERotationMode PreviousRotationMode{ERotationMode::OrientToMovement};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	FGameplayTag PreviousMovementMode{MovementModeTags::Grounded};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	FCharacterInfo PreviousCharacterInfo;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	FGameplayTag PreviousLocomotionAction{FGameplayTag::EmptyTag};

	UPROPERTY(EditAnywhere, Category="PoseSearchData|Choosers", BlueprintReadOnly)
	TObjectPtr<class UChooserTable> LocomotionTable{nullptr};
	UPROPERTY(EditAnywhere, Category="PoseSearchData|Choosers", BlueprintReadOnly)
	TObjectPtr<UChooserTable> StateMachineTable{nullptr};

	UPROPERTY(EditAnywhere, Category="PoseSearchData|Trajectory", BlueprintReadOnly, Transient)
	FTransformTrajectory Trajectory{};
	UPROPERTY(EditAnywhere, Category="PoseSearchData|Trajectory", BlueprintReadOnly, Transient)
	FPoseSearchTrajectoryData TrajectoryGenerationData_Idle{};
	UPROPERTY(EditAnywhere, Category="PoseSearchData|Trajectory", BlueprintReadOnly, Transient)
	FPoseSearchTrajectoryData TrajectoryGenerationData_Moving{};

	UPROPERTY(EditAnywhere, Category="Movement|FootPlacement", BlueprintReadOnly, Transient)
	FFootPlacementPlantSettings PlantSettings_Default{};
	UPROPERTY(EditAnywhere, Category="Movement|FootPlacement", BlueprintReadOnly, Transient)
	FFootPlacementPlantSettings PlantSettings_Stops{};
	UPROPERTY(EditAnywhere, Category="Movement|FootPlacement", BlueprintReadOnly, Transient)
	FFootPlacementInterpolationSettings InterpolationSettings_Default{};
	UPROPERTY(EditAnywhere, Category="Movement|FootPlacement", BlueprintReadOnly, Transient)
	FFootPlacementInterpolationSettings InterpolationSettings_Stops{};
	UPROPERTY(VisibleAnywhere, Category="Additive", BlueprintReadOnly, Transient)
	FRotator SpineRotation{FRotator::ZeroRotator};
	UPROPERTY(EditAnywhere, Category="HeldObject", BlueprintReadOnly, Transient)
	FVector RightHandOffset{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, Category="HeldObject", BlueprintReadOnly, Transient)
	FVector LeftHandOffset{FVector::ZeroVector};

public:
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	bool IsStarting() const;
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	bool IsPivoting() const;
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	bool IsMoving() const;
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	bool ShouldTurnInPlace() const;
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	bool ShouldSpin() const;
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	bool JustLanded_Light() const;
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	bool JustLanded_Heavy() const;
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	bool JustTraversed() const;
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	float GetLandVelocity() const;
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	bool PlayLand() const;
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	bool PlayMovingLand() const;
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	float GetTrajectoryTurnAngle() const;
	UFUNCTION(BlueprintPure, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	bool IsEnableSteering() const;

	UFUNCTION(BlueprintPure, Category="Movement|Lean", meta = (BlueprintThreadSafe))
	FVector2D GetLeanAmount() const;
	UFUNCTION(BlueprintPure, Category="Movement|Lean", meta = (BlueprintThreadSafe))
	FVector GetRelativeAcceleration() const;
	UFUNCTION(BlueprintPure, Category="BlendStack", meta = (BlueprintThreadSafe))
	FQuat GetDesiredFacing() const;

protected:
	UFUNCTION(BlueprintCallable, Category = "Runtime", meta = (BlueprintThreadSafe))
	void RefreshMotionMatchingMovement(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Runtime", meta = (BlueprintThreadSafe))
	void RefreshMatchingPostSelection(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Runtime", meta = (BlueprintThreadSafe))
	void RefreshOffsetRoot(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Runtime", meta = (BlueprintThreadSafe))
	void RefreshBlendStack(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Runtime", meta = (BlueprintThreadSafe))
	void RefreshBlendStackMachine(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Runtime", meta = (BlueprintThreadSafe))
	void RefreshEssentialValues(const float DeltaSeconds);
	UFUNCTION(BlueprintCallable, Category = "Runtime", meta = (BlueprintThreadSafe))
	void RefreshRagdollValues(const float DeltaSeconds);
	UFUNCTION(BlueprintCallable, Category = "Runtime", meta = (BlueprintThreadSafe))
	void RefreshTrajectory(float DeltaSeconds);
	UFUNCTION(BlueprintCallable, Category = "Runtime", meta = (BlueprintThreadSafe))
	void RefreshMovementDirection(float DeltaSeconds);

	/**************
	 * Aim Offsets *
	 **************/
	UPROPERTY(EditAnywhere, Category = "MovementInformation|General", BlueprintReadOnly)
	float HeavyLandSpeedThreshold{700.f};
	UPROPERTY(EditAnywhere, Category = "MovementInformation|General", BlueprintReadOnly)
	bool bLanded{false};
	UPROPERTY(EditAnywhere, Category = "MovementInformation|General", BlueprintReadOnly,
		meta=(EditCondition="!bUseExperimentalStateMachine"))
	int32 MMDatabaseLOD{0};
	UPROPERTY(EditAnywhere, Category = "MovementInformation|General", BlueprintReadOnly)
	uint8 bOffsetRootBoneEnabled : 1 {true};
	UPROPERTY(EditAnywhere, Category = "MovementInformation|General", BlueprintReadOnly)
	uint8 bUseExperimentalStateMachine : 1 {false};

public:
	UFUNCTION(BlueprintPure, Category = "AimOffset", meta = (BlueprintThreadSafe))
	bool IsEnabledAO() const;
	UFUNCTION(BlueprintPure, Category = "AimOffset", meta = (BlueprintThreadSafe))
	FVector2D GetAOValue() const;

	UFUNCTION()
	void OnLanded(const FHitResult& HitResult);

	UFUNCTION(BlueprintPure, Category = "BlendStack", meta = (BlueprintThreadSafe))
	EPoseSearchInterruptMode GetMatchingInterruptMode() const;
	UFUNCTION(BlueprintPure, Category = "BlendStack", meta = (BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootRotationMode() const;
	UFUNCTION(BlueprintPure, Category = "BlendStack", meta = (BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootTranslationMode() const;
	UFUNCTION(BlueprintPure, Category = "BlendStack", meta = (BlueprintThreadSafe))
	float GetOffsetRootTranslationHalfLife() const;
	UFUNCTION(BlueprintPure, Category = "BlendStack", meta = (BlueprintThreadSafe))
	EOrientationWarpingSpace GetOrientationWarpingSpace() const;
	UFUNCTION(BlueprintPure, Category = "AimOffset", meta = (BlueprintThreadSafe))
	float GetAOYaw() const;

	UFUNCTION(BlueprintPure, Category = "BlendStack", meta = (BlueprintThreadSafe))
	FTransform GetHandIKTransform(const FName HandIKSocketName, const FName ObjectIKSocketName,
	                              const FVector& SocketOffset) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PoseSearchData|Trajectory", Transient)
	FPoseSearchTrajectory_WorldCollisionResults CollisionResults{};

	/********************
	 * Overlay blending *
	 *******************/
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshOverlaySettings(float DeltaTime);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshLayering(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Additive|Layering", BlueprintReadOnly)
	FLayeringState LayeringState;

public:
	UGASPAnimInstance() = default;

	virtual void NativeBeginPlay() override;
	virtual void NativeInitializeAnimation() override;
	virtual void PreUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "BlendStack", meta = (BlueprintThreadSafe))
	float GetMatchingNotifyRecencyTimeOut() const;
	UFUNCTION(BlueprintPure, Category = "BlendStack", meta = (BlueprintThreadSafe))
	float GetMatchingBlendTime() const;
	UFUNCTION(BlueprintPure, Category = "BlendStack", meta = (BlueprintThreadSafe))
	FFloatInterval GetMatchingPlayRate() const;
	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FFootPlacementPlantSettings GetPlantSettings() const;
	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FFootPlacementInterpolationSettings GetPlantInterpolationSettings() const;

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FORCEINLINE FCharacterInfo GetCharacterInfo() const { return CharacterInfo; }

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FORCEINLINE EGait GetGait() const { return Gait; }

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FORCEINLINE FGameplayTag GetStanceMode() const { return StanceMode; }

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FORCEINLINE EMovementState GetMovementState() const { return MovementState; }

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FORCEINLINE ERotationMode GetRotationMode() const { return RotationMode; }

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FORCEINLINE FGameplayTag GetMovementMode() const { return MovementMode; }

	FPoseSnapshot& SnapshotFinalRagdollPose();

	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	bool bNotifyTransition_ReTransition{false};
	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	bool bNotifyTransition_ToLoop{false};

protected:
	UPROPERTY(EditAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	TObjectPtr<UAnimSequenceBase> StrafeCurveAnimationAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StateMachine", Transient)
	FGASPBlendStackInputs PreviousBlendStackInputs{};
	UPROPERTY(EditAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	EStateMachineState StateMachineState{EStateMachineState::IdleLoop};
	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	bool bNoValidAnim{true};
	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	float SearchCost{.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StateMachine", Transient)
	EMovementDirection PreviousMovementDirection{};
	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	EMovementDirectionBias MovementDirectionBias{EMovementDirectionBias::LeftFootForward};
	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	FRotator TargetRotation{FRotator::ZeroRotator};
	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	FRotator TargetRotationOnTransitionStart{FRotator::ZeroRotator};
	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	float TargetRotationDelta{0.f};

	UFUNCTION(BlueprintCallable, Category = "StateMachine", meta = (BlueprintThreadSafe))
	void SetBlendStackAnimFromChooser(const FAnimNodeReference& Node, const EStateMachineState NewState,
	                                  const bool bForceBlend = false);

	UFUNCTION(BlueprintPure, Category = "StateMachine", meta = (BlueprintThreadSafe))
	bool IsAnimationAlmostComplete() const;
	UFUNCTION(BlueprintPure, Category = "StateMachine", meta = (BlueprintThreadSafe))
	float GetDynamicPlayRate(const FAnimNodeReference& Node) const;

	UFUNCTION(BlueprintCallable, Category = "StateMachine", meta = (BlueprintThreadSafe))
	void OnStateEntryIdleLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "StateMachine", meta = (BlueprintThreadSafe))
	void OnStateEntryTransitionToIdleLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "StateMachine", meta = (BlueprintThreadSafe))
	void OnStateEntryLocomotionLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "StateMachine", meta = (BlueprintThreadSafe))
	void OnStateEntryTransitionToLocomotionLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "StateMachine", meta = (BlueprintThreadSafe))
	void OnUpdateTransitionToLocomotionLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "StateMachine", meta = (BlueprintThreadSafe))
	void OnStateEntryInAirLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "StateMachine", meta = (BlueprintThreadSafe))
	void OnStateEntryTransitionToInAirLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "StateMachine", meta = (BlueprintThreadSafe))
	void OnStateEntryIdleBreak(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "StateMachine", meta = (BlueprintThreadSafe))
	void RefreshTargetRotation();
	UFUNCTION(BlueprintPure, Category = "StateMachine", meta = (BlueprintThreadSafe))
	float GetStrafeYawRotationOffset() const;
	UFUNCTION(BlueprintCallable, Category = "StateMachine", meta = (BlueprintThreadSafe))
	void UpdateAnimationLoopingFlag(FGASPBlendStackInputs& Inputs) const;
};
