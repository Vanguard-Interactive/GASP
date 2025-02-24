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

public:
	UFUNCTION(BlueprintGetter)
	FORCEINLINE TSubclassOf<UAnimInstance> GetOverlayAnimInstance() const
	{
		return OverlayAnimInstance;
	}
};
