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
	UPROPERTY(BlueprintReadOnly, Category = "References", Transient)
	TWeakObjectPtr<class AGASPCharacter> CachedCharacter{};

	UPROPERTY(BlueprintReadOnly, Category = "References", Transient)
	TWeakObjectPtr<class UGASPCharacterMovementComponent> CachedMovement{};

	/******************
	 * Character state *
	 ******************/
	UPROPERTY(VisibleAnywhere, Category = "CharacterInformation|General", BlueprintReadOnly, Transient)
	FAnimCurves AnimNames{};

	UPROPERTY(VisibleAnywhere, Category = "CharacterInformation|General", BlueprintReadOnly, Transient)
	EMovementDirection MovementDirection{EMovementDirection::F};
	UPROPERTY(VisibleAnywhere, Category = "LocomotionAction", BlueprintReadOnly, Transient)
	FGameplayTag LocomotionAction{LocomotionActionTags::None};

	UPROPERTY(VisibleAnywhere, Category = "CharacterInfromation|States", BlueprintReadOnly, Transient)
	FGait Gait{};
	UPROPERTY(VisibleAnywhere, Category = "CharacterInfromation|States", BlueprintReadOnly, Transient)
	FStanceMode StanceMode{};
	UPROPERTY(VisibleAnywhere, Category = "CharacterInfromation|States", BlueprintReadOnly, Transient)
	FMovementState MovementState{};
	UPROPERTY(VisibleAnywhere, Category = "CharacterInfromation|States", BlueprintReadOnly, Transient)
	FRotationMode RotationMode{};
	UPROPERTY(VisibleAnywhere, Category = "CharacterInfromation|States", BlueprintReadOnly, Transient)
	FMovementMode MovementMode{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|General", Transient)
	FCharacterInfo CharacterInfo{};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|General", Transient)
	FGASPBlendStackInputs BlendStackInputs{};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|General", Transient)
	FMovementDirectionThreshold MovementDirectionThreshold;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionMatching", Transient)
	FMotionMatchingInfo MotionMatching;
	UPROPERTY(VisibleAnywhere, Category = "LocomotionAction|Information", BlueprintReadOnly, Transient)
	FRagdollingAnimationState RagdollingState;
	UPROPERTY(VisibleAnywhere, Category = "Additive|Poses", BlueprintReadOnly, Transient)
	FGASPBlendPoses BlendPoses;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	FStanceMode PreviousStanceMode;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	FGait PreviousGait;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	FMovementState PreviousMovementState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	FRotationMode PreviousRotationMode;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	FMovementMode PreviousMovementMode;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	FCharacterInfo PreviousCharacterInfo;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterInformation|PreviousValues", Transient)
	FGameplayTag PreviousLocomotionAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	FGameplayTagContainer OverlayMode{OverlayModeTags::Default};

	UPROPERTY(VisibleAnywhere, Category="PoseSearchData|Choosers", BlueprintReadWrite, Transient)
	TSoftObjectPtr<class UChooserTable> LocomotionTable{};
	UPROPERTY(VisibleAnywhere, Category="PoseSearchData|Choosers", BlueprintReadWrite, Transient)
	TSoftObjectPtr<UChooserTable> OverlayTable{};
	UPROPERTY(VisibleAnywhere, Category="PoseSearchData|Choosers", BlueprintReadWrite, Transient)
	TSoftObjectPtr<UChooserTable> StateMachineTable{};

	UPROPERTY(VisibleAnywhere, Category="PoseSearchData|Trajectory", BlueprintReadWrite, Transient)
	FPoseSearchQueryTrajectory Trajectory{};
	UPROPERTY(VisibleAnywhere, Category="PoseSearchData|Trajectory", BlueprintReadWrite, Transient)
	FPoseSearchTrajectoryData TrajectoryGenerationData_Idle{};
	UPROPERTY(VisibleAnywhere, Category="PoseSearchData|Trajectory", BlueprintReadWrite, Transient)
	FPoseSearchTrajectoryData TrajectoryGenerationData_Moving{};

	UPROPERTY(VisibleAnywhere, Category="Movement|FootPlacement", BlueprintReadOnly, Transient)
	FFootPlacementPlantSettings PlantSettings_Default{};
	UPROPERTY(VisibleAnywhere, Category="Movement|FootPlacement", BlueprintReadOnly, Transient)
	FFootPlacementPlantSettings PlantSettings_Stops{};
	UPROPERTY(VisibleAnywhere, Category="Movement|FootPlacement", BlueprintReadOnly, Transient)
	FFootPlacementInterpolationSettings InterpolationSettings_Default{};
	UPROPERTY(VisibleAnywhere, Category="Movement|FootPlacement", BlueprintReadOnly, Transient)
	FFootPlacementInterpolationSettings InterpolationSettings_Stops{};
	UPROPERTY(VisibleAnywhere, Category="Additive", BlueprintReadOnly, Transient)
	FRotator SpineRotation{FRotator::ZeroRotator};

public:
	UFUNCTION(BlueprintPure, Category="Movement|Analys", meta = (BlueprintThreadSafe))
	bool IsStarting() const;
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool IsPivoting() const;
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

	UFUNCTION(BlueprintPure, Category="Leaning", meta = (BlueprintThreadSafe))
	FVector2D GetLeanAmount() const;
	UFUNCTION(BlueprintPure, Category="Leaning", meta = (BlueprintThreadSafe))
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
	void RefreshEssentialValues(const float DeltaSeconds);
	UFUNCTION(BlueprintCallable, Category = "Runtime", meta = (BlueprintThreadSafe))
	void RefreshRagdollValues(const float DeltaSeconds);
	UFUNCTION(BlueprintCallable, Category = "Runtive", meta = (BlueprintThreadSafe))
	virtual void RefreshCVar();
	UFUNCTION(BlueprintCallable, Category = "Runtive", meta = (BlueprintThreadSafe))
	void RefreshTrajectory(float DeltaSeconds);
	UFUNCTION(BlueprintCallable, Category = "Runtive", meta = (BlueprintThreadSafe))
	void RefreshMovementDirection(float DeltaSeconds);

	/**************
	 * Aim Offsets *
	 **************/
	UPROPERTY(EditAnywhere, Category = "MovementInformation|General", BlueprintReadOnly)
	float HeavyLandSpeedThreshold{700.f};
	UPROPERTY(EditAnywhere, Category = "MovementInformation|General", BlueprintReadOnly)
	bool bLanded{false};
	UPROPERTY(EditAnywhere, Category = "MovementInformation|General", BlueprintReadOnly)
	int32 MMDatabaseLOD{0};
	UPROPERTY(EditAnywhere, Category = "MovementInformation|General", BlueprintReadOnly)
	uint8 OffsetRootBoneEnabled : 1 {false};

public:
	UFUNCTION(BlueprintPure, Category = "AimOffset", meta = (BlueprintThreadSafe))
	bool IsEnabledAO() const;
	UFUNCTION(BlueprintPure, Category = "AimOffset", meta = (BlueprintThreadSafe))
	FVector2D GetAOValue() const;

	UFUNCTION()
	void OnLanded(const FHitResult& HitResult);

	UFUNCTION()
	void OnOverlayModeChanged(const FGameplayTag OldOverlayMode);

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
	UPROPERTY(EditAnywhere, Category = "Additive|Layering", BlueprintReadOnly)
	FLayeringNames LayeringCurveNames;

public:
	UGASPAnimInstance() = default;

	virtual void NativeBeginPlay() override;
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void PreUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "BlendStack", meta = (BlueprintThreadSafe))
	float GetMatchingNotifyRecencyTimeOut() const;
	UFUNCTION(BlueprintPure, Category = "BlendStack", meta = (BlueprintThreadSafe))
	float GetMatchingBlendTime() const;
	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FFootPlacementPlantSettings GetPlantSettings() const;
	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FFootPlacementInterpolationSettings GetPlantInterpolationSettings() const;

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FMovementDirectionThreshold GetMovementDirectionThresholds() const;

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FORCEINLINE FCharacterInfo GetCharacterInfo() const { return CharacterInfo; }

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FORCEINLINE FGait GetGait() const { return Gait; }

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FORCEINLINE FStanceMode GetStanceMode() const { return StanceMode; }

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FORCEINLINE FMovementState GetMovementState() const { return MovementState; }

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FORCEINLINE FRotationMode GetRotationMode() const { return RotationMode; }

	UFUNCTION(BlueprintGetter, Category = "Movement|Analys", meta = (BlueprintThreadSafe))
	FORCEINLINE FMovementMode GetMovementMode() const { return MovementMode; }

	FPoseSnapshot& SnapshotFinalRagdollPose();

	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	bool bNotifyTransition_ReTransition{false};
	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	bool bNotifyTransition_ToLoop{false};

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StateMachine", Transient)
	FGASPBlendStackInputs PreviousBlendStackInputs{};
	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	EStateMachineState StateMachineState{};
	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	bool bNoValidAnim{true};
	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	float SearchCost{.0f};
	UPROPERTY(VisibleAnywhere, Category = "StateMachine", BlueprintReadOnly, Transient)
	TObjectPtr<UAnimSequenceBase> StrafeCurveAnimationAsset;
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
	void SetBlendStackAnimFromChooser(const FAnimNodeReference& Node, EStateMachineState NewState,
	                                  bool bForceBlend = false);

	UFUNCTION(BlueprintPure, Category = "StateMachine", meta = (BlueprintThreadSafe, DefaultToSelf))
	bool IsAnimationAlmostComplete();

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
	UFUNCTION(BlueprintPure, Category = "StateMachine", meta = (BlueprintThreadSafe))
	static bool GetCurveValueSafe(const UAnimSequence* AnimSequence, const FName& CurveName, float Time,
	                              float& OutValue);
};
