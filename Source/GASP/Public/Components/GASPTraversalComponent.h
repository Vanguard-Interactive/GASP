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
	FGameplayTag ActionType{FGameplayTag::EmptyTag};
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
	FGameplayTag ActionType;
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

	FTraversalResult(const bool TraversalCheckFailed = false, const bool MontageSelectionFailed = false)
	{
		bTraversalCheckFailed = TraversalCheckFailed;
		bMontageSelectionFailed = MontageSelectionFailed;
	}
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASP_API UGASPTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category="Traversal")
	FTimerHandle TraversalEndHandle;

public:
	// Sets default values for this component's properties
	UGASPTraversalComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** 
	 * Processes motion warping for a hurdle movement
	 * @param CurveName - Name of the curve to retrieve values from
	 * @param WarpTarget - The motion warping target name
	 * @param Value - The retrieved value from the animation curve
	 */
	void ProccessHurdle(const FName CurveName, const FName WarpTarget, float& Value) const;

	/** 
	 * Updates motion warping targets based on traversal results 
	 */
	UFUNCTION(BlueprintCallable, Category="Traversal")
	void UpdateWarpTargets();

	/** 
	 * Handles traversal action replication on the server 
	 * @param TraversalRep - The replicated traversal result data
	 */
	UFUNCTION(BlueprintCallable, Category="Traversal")
	void Traversal_ServerImplementation(const FTraversalCheckResult TraversalRep);

	/** 
	 * Called when a traversal action starts 
	 */
	UFUNCTION(BlueprintCallable, Category="Traversal")
	void OnTraversalStart();

	/** 
	 * Replication callback for TraversalCheckResult 
	 */
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void OnRep_TraversalResult();

	/** 
	 * Called when a traversal action ends 
	 */
	UFUNCTION(BlueprintCallable, Category="Traversal")
	void OnTraversalEnd() const;

	/** 
	 * Handles the completion of a traversal action 
	 * @param NotifyName - The name of the animation notify that triggered the completion event
	 */
	UFUNCTION()
	void OnCompleteTraversal(FName NotifyName);

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category="Traversal", ReplicatedUsing="OnRep_TraversalResult", Transient)
	FTraversalCheckResult TraversalCheckResult{};

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category="Traversal", Transient)
	uint8 bDoingTraversalAction : 1{false};

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Traversal")
	float IgnoreCorrectionDelay{.2f};

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Traversal")
	TSoftObjectPtr<class UChooserTable> TraversalAnimationsChooserTable;

public:
	/** 
	* Returns a pre-configured collision query for traversal traces 
	* @return Configured collision query parameters 
	*/
	FCollisionQueryParams GetQueryParams() const;

	/** 
	 * Performs a traversal action check and returns the result 
	 * @param CheckInputs - The inputs required to perform the traversal check
	 * @return The result of the traversal check
	 */
	UFUNCTION(BlueprintCallable, Category="Traversal", meta = (BlueprintThreadSafe))
	FTraversalResult TryTraversalAction(FTraversalCheckInputs CheckInputs);


	/** 
	 * Executes the traversal action (native event for blueprint extension) 
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Traversal", meta = (BlueprintThreadSafe))
	void PerformTraversalAction();

	/** 
	 * Handles traversal execution on the server 
	 * @param TraversalRep - The traversal result data to be replicated
	 */
	UFUNCTION(Reliable, Server, Category="Traversal")
	void Server_Traversal(FTraversalCheckResult TraversalRep);

	/** 
	 * Checks whether the character is currently performing a traversal action 
	 * @return True if the character is in a traversal action, otherwise false
	 */
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
