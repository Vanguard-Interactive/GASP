// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Audio/FoleyAudioBankPrimaryDataAsset.h"
#include "UObject/Object.h"
#include "FoleyAudioBankInterface.generated.h"

/**
 * 
 */
UINTERFACE()
class GASP_API UFoleyAudioBankInterface : public UInterface
{
	GENERATED_BODY()
};

class IFoleyAudioBankInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(Blueprintable, Category="Foley|Audio")
	virtual UFoleyAudioBankPrimaryDataAsset* GetFoleyAudioBank() = 0;

	UFUNCTION(Blueprintable, Category="Foley|Audio")
	virtual bool CanPlayFootstepSounds() = 0;
};
