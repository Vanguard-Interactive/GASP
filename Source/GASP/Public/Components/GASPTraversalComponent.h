// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Types/EnumTypes.h"
#include "Types/GameplayTags.h"
#include "Types/StructTypes.h"
#include "GASPTraversalComponent.generated.h"


class UGASPAnimInstance;
class UCapsuleComponent;
class USplineComponent;
class UMotionWarpingComponent;
class UGASPCharacterMovementComponent;
class AGASPCharacter;

USTRUCT(BlueprintType)
struct GASP_API FTraversalChooserInput
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FGameplayTagContainer ActionType{LocomotionActionTags::None};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	EGait Gait{EGait::Walk};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	TEnumAsByte<EMovementMode> MovementMode{MOVE_None};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float Speed{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float ObstacleHeight{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float ObstacleDepth{0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float BackLedgeHeight{0.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	uint8 bHasFrontLedge : 1{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	uint8 bHasBackLedge : 1{false};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	uint8 bHasBackFloor : 1{false};
};

USTRUCT(BlueprintType)
struct GASP_API FTraversalChooserOutput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ActionType;
};

USTRUCT(BlueprintType)
struct GASP_API FTraversalCheckInputs
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector TraceForwardDirection{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float TraceForwardDistance{0.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector TraceOriginOffset{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector TraceEndOffset{FVector::ZeroVector};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float TraceRadius{0.f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float TraceHalfHeight{0.f};
};

USTRUCT(BlueprintType)
struct GASP_API FTraversalResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	bool bTraversalCheckFailed{false};
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	bool bMontageSelectionFailed{false};

	explicit FTraversalResult(const bool TraversalCheckFailed = false, const bool MontageSelectionFailed = false)
	{
		bTraversalCheckFailed = TraversalCheckFailed;
		bMontageSelectionFailed = MontageSelectionFailed;
	}
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASP_API UGASPTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGASPTraversalComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void ProccessHurdle(const FName CurveName, const FName WarpTarget, float& Value) const;

	UFUNCTION(BlueprintCallable)
	void UpdateWarpTargets();

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, Category="Traversal")
	void Traversal_ServerImplementation(const FTraversalCheckResult TraversalRep);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, Category="Traversal")
	void OnTraversalStart();

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void OnRep_TraversalResult();

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, Category="Traversal")
	void OnTraversalEnd() const;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category="Traversal", ReplicatedUsing="OnRep_TraversalResult", Transient)
	FTraversalCheckResult TraversalCheckResult{};

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category="Traversal", Transient)
	uint8 bDoingTraversalAction : 1{false};

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Traversal")
	float IgnoreCorrectionDelay{.2f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Traversal")
	TSoftObjectPtr<class UChooserTable> TraversalAnimationsChooserTable;

	UFUNCTION()
	void OnCompleteTraversal(FName NotifyName);

public:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	FTraversalResult TryTraversalAction(FTraversalCheckInputs CheckInputs);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, Category="Traversal")
	void PerformTraversalAction();

	/** Please add a function description */
	UFUNCTION(Reliable, Server, Category="Traversal")
	void Server_Traversal(FTraversalCheckResult TraversalRep);

	/** Please add a function description */
	UFUNCTION(BlueprintPure, Category="Traversal")
	bool IsDoingTraversal() const;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AGASPCharacter> CharacterOwner{};

	UPROPERTY(Transient)
	TWeakObjectPtr<UGASPCharacterMovementComponent> MovementComponent{};

	UPROPERTY(Transient)
	TWeakObjectPtr<UMotionWarpingComponent> MotionWarpingComponent{};

	UPROPERTY(Transient)
	TWeakObjectPtr<UCapsuleComponent> CapsuleComponent{};

	UPROPERTY(Transient)
	TWeakObjectPtr<USkeletalMeshComponent> MeshComponent{};

	UPROPERTY(Transient)
	TWeakObjectPtr<UGASPAnimInstance> AnimInstance{};
};
