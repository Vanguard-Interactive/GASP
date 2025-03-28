// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GASPBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class GASP_API UGASPBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UGASPBlueprintLibrary() = default;

	UFUNCTION(BlueprintPure, Category = "GASP|Utility",
		meta = (DefaultToSelf = "Character", AutoCreateRefTerm = "CurveName", ReturnDisplayName = "Curve Value"))
	static float GetAnimationCurveValueFromCharacter(const ACharacter* Character, const FName& CurveName);


	UFUNCTION(BlueprintPure, Category = "GASP|Utility",
		meta = (AutoCreateRefTerm = "Tag", ReturnDisplayName = "Tag Name"))
	static FName GetShortTagName(const FGameplayTag& GameplayTag);

	UFUNCTION(BlueprintPure, Category = "GASP|Utility",
		meta = (AutoCreateRefTerm = "GameplayTag", ReturnDisplayName = "All Child Tags"))
	static FGameplayTagContainer GetAllChildTags(const FGameplayTag& GameplayTag);
};
