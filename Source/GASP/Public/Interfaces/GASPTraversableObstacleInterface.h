// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/StructTypes.h"
#include "GASPTraversableObstacleInterface.generated.h"

/**
 * 
 */
UINTERFACE(Blueprintable)
class UGASPTraversableObstacleInterface : public UInterface
{
	GENERATED_BODY()
};


class GASP_API IGASPTraversableObstacleInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Traversal")
	void GetLedgeTransforms(const FVector& HitLocation, const FVector& ActorLocation,
	                        UPARAM(ref, DisplayName = "TraversalTraceResult") FTraversalCheckResult&
	                        TraversalTraceResultOut) const;
};
