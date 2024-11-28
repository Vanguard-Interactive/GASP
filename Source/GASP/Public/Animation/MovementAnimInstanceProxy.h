// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstanceProxy.h"
#include "MovementAnimInstanceProxy.generated.h"

/**
 * 
 */
USTRUCT()
struct FMovementAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FMovementAnimInstanceProxy() = default;

	explicit FMovementAnimInstanceProxy(UAnimInstance* InAnimInstance);

protected:
	virtual void Update(float DeltaSeconds) override;


	//UFUNCTION(meta = (BlueprintThreadSafe))
	//bool isStarting() const;
	//UFUNCTION(meta = (BlueprintThreadSafe))
	//bool isPivoting() const;
	//UFUNCTION(meta = (BlueprintThreadSafe))
	//bool ShouldTurnInPlace() const;
	//UFUNCTION(meta = (BlueprintThreadSafe))
	//bool ShouldSpin() const;
};