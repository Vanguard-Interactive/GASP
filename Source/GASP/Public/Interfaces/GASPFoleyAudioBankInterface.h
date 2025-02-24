// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Audio/GASPFoleyAudioBankPrimaryDataAsset.h"
#include "UObject/Object.h"
#include "GASPFoleyAudioBankInterface.generated.h"

/**
 * 
 */
UINTERFACE()
class UGASPFoleyAudioBankInterface : public UInterface
{
	GENERATED_BODY()
};

class GASP_API IGASPFoleyAudioBankInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Foley|Audio")
	UGASPFoleyAudioBankPrimaryDataAsset* GetFoleyAudioBank();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Foley|Audio")
	bool CanPlayFootstepSounds();
};
