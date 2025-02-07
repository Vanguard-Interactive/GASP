// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FoleyAudioBankPrimaryDataAsset.generated.h"

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class GASP_API UFoleyAudioBankPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foley|Audio", meta=(AllowPrivateAccess))
	TMap<FGameplayTag, TSoftObjectPtr<USoundBase>> FoleyPrimaryData;

public:
	UFUNCTION(BlueprintPure, Category = "Foley|Audio")
	USoundBase* GetSoundFromEvent(FGameplayTag Event);
};
