// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Types/EnumTypes.h"
#include "AnimNotifyState_EarlyTransition.generated.h"

/**
 * 
 */
UCLASS()
class GASP_API UAnimNotifyState_EarlyTransition : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EarlyTransition")
	EEarlyTransitionDestination TransitionDestination{EEarlyTransitionDestination::ReTransition};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EarlyTransition")
	EEarlyTransitionCondition TransitionCondition{EEarlyTransitionCondition::Always};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EarlyTransition")
	EGait GaitNotEqual{EGait::Walk};

public:
	UAnimNotifyState_EarlyTransition();

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	                        const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
