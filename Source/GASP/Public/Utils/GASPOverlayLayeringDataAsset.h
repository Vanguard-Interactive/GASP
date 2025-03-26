// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GASPOverlayLayeringDataAsset.generated.h"

/**
 *
 */
UCLASS()
class GASP_API UGASPOverlayLayeringDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> OverlayAnimInstance;

	// Maybe use TArray, but how, get target value from it
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector RightHandCorrection{FVector::ZeroVector};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector LeftHandCorrection{FVector::ZeroVector};

	// UPROPERTY(EditAnywhere, BlueprintReadOnly)
	// TArray<FVector> HandCorrections;

public:
	UFUNCTION(BlueprintGetter)
	FORCEINLINE TSubclassOf<UAnimInstance> GetOverlayAnimInstance() const
	{
		return OverlayAnimInstance;
	}

	UFUNCTION(BlueprintGetter)
	FORCEINLINE FVector GetLeftHandCorrection() const
	{
		return LeftHandCorrection;
	}

	UFUNCTION(BlueprintGetter)
	FORCEINLINE FVector GetRightHandCorrection() const
	{
		return RightHandCorrection;
	}

	// UFUNCTION(BlueprintGetter)
	// FORCEINLINE TArray<FVector> GetHandCorrections() const
	// {
	// 	return HandCorrection;
	// }
};
