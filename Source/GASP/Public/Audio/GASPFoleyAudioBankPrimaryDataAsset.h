// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StreamableManager.h"
#include "GameplayTagContainer.h"
#include "GASPFoleyAudioBankPrimaryDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class GASP_API UGASPFoleyAudioBankPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

	FStreamableManager StreamableManager;
	TSharedPtr<FStreamableHandle> StreamableHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Foley|Audio")
	TMap<FGameplayTag, TSoftObjectPtr<USoundBase>> FoleyPrimaryData;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSoundsPreloaded);

	UPROPERTY(BlueprintAssignable, Category = "Foley|Audio")
	FOnSoundsPreloaded OnSoundsPreloaded;

	UFUNCTION(BlueprintPure, Category = "Foley|Audio", meta=(AutoCreateRefTerm="Event"))
	TSoftObjectPtr<USoundBase> GetSoundFromEvent(const FGameplayTag Event);

	UFUNCTION(BlueprintCallable, Category = "Foley|Audio")
	void PreloadSoundsAsync();

private:
	void HandleAssetsLoaded();
};
