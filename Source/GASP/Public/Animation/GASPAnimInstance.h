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
    /*************
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
    FGait Gait{};
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
    FAnimCurves AnimNames{};

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
    EMovementDirection MovementDirection{EMovementDirection::F};

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
    FStanceMode StanceMode{};
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
    FMovementState MovementState{};
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
    FRotationMode RotationMode{};
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
    FMovementMode MovementMode{};
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Information", Transient)
    FCharacterInfo CharacterInfo{};
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Information", Transient)
    FBlendStackInputs BlendStackInputs{};
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Information", Transient)
    FMovementDirectionThreshold MovementDirectionThreshold;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching", Transient)
    FMotionMatchingInfo MotionMatching;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
    uint8 bWantsToAim : 1 {false};

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

    UPROPERTY(BlueprintReadOnly, Transient)
    EOverlayState OverlayState{};

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient)
    TObjectPtr<class UChooserTable> LocomotionTable{};
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient)
    TObjectPtr<class UChooserTable> OverlayTable{};

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

    UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
    bool isStarting() const;
    UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
    bool isMoving() const;
    UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
    bool isPivoting() const;
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

    /**************
     * Aim Offsets *
     **************/
    UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
    bool IsEnabledAO() const;
    UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
    FVector2D GetAOValue() const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
    float HeavyLandSpeedThreshold{700.f};
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
    bool bLanded{false};
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
    int32 MMDatabaseLOD{0};
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
    uint8 OffsetRootBoneEnabled : 1 {false};

    UFUNCTION()
    void OnLanded(const FHitResult& HitResult);

    UFUNCTION()
    void OnOverlayStateChanged(EOverlayState NewOverlayState);

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient)
    TWeakObjectPtr<class UOverlayLayeringDataAsset> LayerSettings;

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
    UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
    FFootPlacementPlantSettings GetPlantSettings() const;
    UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
    FFootPlacementInterpolationSettings GetPlantInterpolationSettings() const;

    UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
    FMovementDirectionThreshold GetMovementDirectionThresholds() const;

    FORCEINLINE FCharacterInfo GetCharacterInfo() const
    {
        return CharacterInfo;
    }
};
