// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/GASPTraversableObstacleInterface.h"
#include "Types/StructTypes.h"
#include "UObject/Interface.h"
#include "GASPTraversableObstacleComponent.generated.h"

class USplineComponent;


/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASP_API UGASPTraversableObstacleComponent : public UActorComponent, public IGASPTraversableObstacleInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	TArray<USplineComponent*> Ledges{};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	TMap<USplineComponent*, USplineComponent*> OppositeLedges{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal")
	float MinLedgeWidth = 60.0f;

	UFUNCTION()
	USplineComponent* FindLedgeClosestToActor(const FVector& ActorLocation) const;

public:
	virtual void GetLedgeTransforms_Implementation(const FVector& HitLocation, const FVector& ActorLocation,
	                                               FTraversalCheckResult& TraversalTraceResultOut) const override;

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void Initialize(UPARAM(DisplayName = "Ledges") const TArray<USplineComponent*>& NewLedges,
	                UPARAM(DisplayName = "OppositeLedges") const TMap<USplineComponent*, USplineComponent*>&
	                NewOppositeLedges);

	FORCEINLINE const TArray<USplineComponent*>& GetLedges() const { return Ledges; }
	FORCEINLINE const TMap<USplineComponent*, USplineComponent*>& GetOppositeLedges() const
	{
		return OppositeLedges;
	}

	FORCEINLINE float GetMinLedgeWidth() const { return MinLedgeWidth; }
};
