// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Audio/GASPFoleyAudioBankPrimaryDataAsset.h"
#include "Types/EnumTypes.h"
#include "AnimNotify_FoleyEvent.generated.h"

/**
 * 
 */
UCLASS()
class GASP_API UAnimNotify_FoleyEvent : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimNotify")
	FGameplayTag Event{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimNotify")
	EFoleyEventSide Side{EFoleyEventSide::None};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimNotify")
	float VolumeMultiplier{1.f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimNotify")
	float PitchMultiplier{1.f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimNotify")
	TObjectPtr<UGASPFoleyAudioBankPrimaryDataAsset> DefaultBank{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimNotify|Debug")
	FLinearColor VisLogDebugColor{FLinearColor::Black};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimNotify|Debug")
	FString VisLogDebugText{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimNotify")
	FGameplayTagContainer ActionTags{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimNotify")
	FGameplayTagContainer MovementTags{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimNotify")
	float TraceLength{15.f};

public:
	UAnimNotify_FoleyEvent();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                    const FAnimNotifyEventReference& EventReference) override;


	virtual FString GetNotifyName_Implementation() const override;

	UFUNCTION(Blueprintable, Category="Foley|Audio")
	bool GetFoleyAudioBank(const USkeletalMeshComponent* MeshComponent);
};
