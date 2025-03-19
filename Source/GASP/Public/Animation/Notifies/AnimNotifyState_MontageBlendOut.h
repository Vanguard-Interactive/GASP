// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Types/EnumTypes.h"
#include "AnimNotifyState_MontageBlendOut.generated.h"

/**
 * 
 */
UCLASS()
class GASP_API UAnimNotifyState_MontageBlendOut : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MontageBlendOut")
	ETraversalBlendOutCondition BlendOutCondition{ETraversalBlendOutCondition::ForceBlendOut};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MontageBlendOut")
	float BlendOutTime{.2f};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MontageBlendOut")
	FName BlendProfile{TEXT("FastFeet_InstantRoot")};

public:
	UAnimNotifyState_MontageBlendOut();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	                        const FAnimNotifyEventReference& EventReference) override;
};
