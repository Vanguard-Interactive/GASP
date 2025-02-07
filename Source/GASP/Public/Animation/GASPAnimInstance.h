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
#include "PoseSearch/PoseSearchTrajectoryTypes.h"
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

protected:
	/**************
	 * References *
	 *************/
	UPROPERTY(BlueprintReadOnly, Transient)
	TWeakObjectPtr<class AGASPCharacter> CachedCharacter{};

	UPROPERTY(BlueprintReadOnly, Transient)
	TWeakObjectPtr<class UGASPCharacterMovementComponent> CachedMovement{};

	/******************
	 * Character state *
	 ******************/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	FAnimCurves AnimNames{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	EMovementDirection MovementDirection{EMovementDirection::F};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	ELocomotionAction LocomotionAction{ELocomotionAction::None};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	FGait Gait{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	FStanceMode StanceMode{};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	FMovementState MovementState{};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	FRotationMode RotationMode{};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	FMovementMode MovementMode{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Information", Transient)
	FCharacterInfo CharacterInfo{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Information", Transient)
	FGASPBlendStackInputs BlendStackInputs{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Information", Transient)
	FMovementDirectionThreshold MovementDirectionThreshold;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching", Transient)
	FMotionMatchingInfo MotionMatching;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRagdollingAnimationState RagdollingState;

	UPROPERTY(BlueprintReadOnly, Transient)
	FStanceMode PreviousStanceMode;
	UPROPERTY(BlueprintReadOnly, Transient)
	FGait PreviousGait;
	UPROPERTY(BlueprintReadOnly, Transient)
	FMovementState PreviousMovementState;
	UPROPERTY(BlueprintReadOnly, Transient)
	FRotationMode PreviousRotationMode;
	UPROPERTY(BlueprintReadOnly, Transient)
	FMovementMode PreviousMovementMode;
	UPROPERTY(BlueprintReadOnly, Category = "Character Information", Transient)
	FCharacterInfo PreviousCharacterInfo;
	UPROPERTY(BlueprintReadOnly, Category = "Character Information", Transient)
	ELocomotionAction PreviousLocomotionAction;

	UPROPERTY(BlueprintReadOnly, Transient)
	EOverlayState OverlayState{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient)
	TObjectPtr<class UChooserTable> LocomotionTable{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient)
	TObjectPtr<UChooserTable> OverlayTable{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient)
	TObjectPtr<UChooserTable> StateMachineTable{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient)
	FPoseSearchQueryTrajectory Trajectory{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient)
	FPoseSearchTrajectoryData TrajectoryGenerationData_Idle{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient)
	FPoseSearchTrajectoryData TrajectoryGenerationData_Moving{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient)
	FFootPlacementPlantSettings PlantSettings_Default{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient)
	FFootPlacementPlantSettings PlantSettings_Stops{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient)
	FFootPlacementInterpolationSettings InterpolationSettings_Default{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient)
	FFootPlacementInterpolationSettings InterpolationSettings_Stops{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	FRotator SpineRotation{FRotator::ZeroRotator};

public:
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool IsStarting() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool IsPivoting() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool ShouldTurnInPlace() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool ShouldSpin() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool JustLanded_Light() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool JustLanded_Heavy() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool JustTraversed() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetLandVelocity() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool PlayLand() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool PlayMovingLand() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetTrajectoryTurnAngle() const;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	FVector2D GetLeanAmount() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	FVector GetRelativeAcceleration() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	FQuat GetDesiredFacing() const;

protected:
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshMotionMatchingMovement(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshMatchingPostSelection(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshOffsetRoot(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshBlendStack(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshEssentialValues(const float DeltaSeconds);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshRagdollValues(const float DeltaSeconds);

	/**************
	 * Aim Offsets *
	 **************/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	float HeavyLandSpeedThreshold{700.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	bool bLanded{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	int32 MMDatabaseLOD{0};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	uint8 OffsetRootBoneEnabled : 1 {false};

public:
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool IsEnabledAO() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	FVector2D GetAOValue() const;

	UFUNCTION()
	void OnLanded(const FHitResult& HitResult);

	UFUNCTION()
	void OnOverlayStateChanged(const EOverlayState NewOverlayState);

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	EPoseSearchInterruptMode GetMatchingInterruptMode() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootRotationMode() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootTranslationMode() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetOffsetRootTranslationHalfLife() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	EOrientationWarpingSpace GetOrientationWarpingSpace() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetAOYaw() const;

protected:
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool IsEnableSteering() const;
	UPROPERTY(BlueprintReadWrite, Transient)
	FPoseSearchTrajectory_WorldCollisionResults CollisionResults{};

	/********************
	 * Overlay blending *
	 *******************/
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshOverlaySettings(float DeltaTime);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshLayering();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	FLayeringState LayeringState;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	FLayeringNames LayeringCurveNames;

public:
	UGASPAnimInstance() = default;

	virtual void NativeBeginPlay() override;
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void PreUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	virtual void RefreshCVar();

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshTrajectory(float DeltaSeconds);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshMovementDirection(float DeltaSeconds);

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetMatchingNotifyRecencyTimeOut() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetMatchingBlendTime() const;
	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FFootPlacementPlantSettings GetPlantSettings() const;
	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FFootPlacementInterpolationSettings GetPlantInterpolationSettings() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FMovementDirectionThreshold GetMovementDirectionThresholds() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FORCEINLINE FCharacterInfo GetCharacterInfo() const { return CharacterInfo; }

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FORCEINLINE FGait GetGait() const { return Gait; }

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FORCEINLINE FStanceMode GetStanceMode() const { return StanceMode; }

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FORCEINLINE FMovementState GetMovementState() const { return MovementState; }

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FORCEINLINE FRotationMode GetRotationMode() const { return RotationMode; }

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FORCEINLINE FMovementMode GetMovementMode() const { return MovementMode; }

	FPoseSnapshot& SnapshotFinalRagdollPose();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	bool bNotifyTransition_ReTransition{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	bool bNotifyTransition_ToLoop{false};

protected:
	UPROPERTY(BlueprintReadOnly, Transient)
	FGASPBlendStackInputs PreviousBlendStackInputs{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	EStateMachineState StateMachineState{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	bool bNoValidAnim{true};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	float SearchCost{.0f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	TObjectPtr<UAnimSequenceBase> StrafeCurveAnimationAsset;
	UPROPERTY(BlueprintReadOnly, Transient)
	EMovementDirection PreviousMovementDirection{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	EMovementDirectionBias MovementDirectionBias{EMovementDirectionBias::LeftFootForward};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	FRotator TargetRotation{FRotator::ZeroRotator};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	FRotator TargetRotationOnTransitionStart{FRotator::ZeroRotator};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
	float TargetRotationDelta{0.f};

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetBlendStackAnimFromChooser(const FAnimNodeReference& Node, EStateMachineState NewState,
	                                  bool bForceBlend = false);

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, DefaultToSelf))
	bool IsAnimationAlmostComplete();

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetDynamicPlayRate(const FAnimNodeReference& Node) const;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void OnStateEntryIdleLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void OnStateEntryTransitionToIdleLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void OnStateEntryLocomotionLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void OnStateEntryTransitionToLocomotionLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void OnUpdateTransitionToLocomotionLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void OnStateEntryInAirLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void OnStateEntryTransitionToInAirLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void OnStateEntryIdleBreak(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void RefreshTargetRotation();
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetStrafeYawRotationOffset() const;
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateAnimationLoopingFlag(FGASPBlendStackInputs& Inputs) const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	static bool GetCurveValueSafe(const UAnimSequence* AnimSequence, const FName& CurveName, float Time,
	                              float& OutValue);
};
