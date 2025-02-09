// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StreamableManager.h"
#include "FoleyAudioBankPrimaryDataAsset.generated.h"

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class GASP_API UFoleyAudioBankPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

	FStreamableManager StreamableManager;

	UPROPERTY(EditDefaultsOnly, Category = "Foley|Audio")
	TMap<FGameplayTag, TSoftObjectPtr<USoundBase>> FoleyPrimaryData;


	// Храним ссылку на handle загрузки
	TSharedPtr<FStreamableHandle> StreamableHandle;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSoundsPreloaded);

	UPROPERTY(BlueprintAssignable, Category = "Foley|Audio")
	FOnSoundsPreloaded OnSoundsPreloaded;

	UFUNCTION(BlueprintPure, Category = "Foley|Audio", meta=(AutoCreateRefTerm="Event"))
	USoundBase* GetSoundFromEvent(const FGameplayTag Event) const;

	UFUNCTION(BlueprintCallable, Category = "Foley|Audio")
	void PreloadSoundsAsync();

private:
	// Callback для завершения загрузки
	void HandleAssetsLoaded();
};
