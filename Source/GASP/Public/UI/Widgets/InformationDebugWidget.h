// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/StructTypes.h"
#include "InformationDebugWidget.generated.h"

/**
 * 
 */
UCLASS()
class GASP_API UInformationDebugWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Information Debug Widget")
	TWeakObjectPtr<class AGASPCharacter> CachedCharacter;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Information Debug Widget")
	TWeakObjectPtr<class UGASPAnimInstance> AnimInstance;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Information Debug Widget")
	FCharacterInfo CharacterInfo;
public:
	
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void UpdateCharacterInfo();
};
