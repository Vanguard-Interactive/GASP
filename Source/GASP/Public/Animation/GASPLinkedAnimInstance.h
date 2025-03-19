// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Types/StructTypes.h"
#include "GASPLinkedAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class GASP_API UGASPLinkedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category = "State", Transient)
	TWeakObjectPtr<class UGASPAnimInstance> Parent;

	UPROPERTY(VisibleAnywhere, Category = "State", Transient)
	TObjectPtr<class AGASPCharacter> Character;

public:
	UGASPLinkedAnimInstance();

	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintPure, meta = (HideSelfPin, BlueprintThreadSafe, ReturnDisplayName = "Parent"))
	UGASPAnimInstance* GetParent() const;

	UFUNCTION(BlueprintPure, meta = (HideSelfPin, BlueprintThreadSafe, ReturnDisplayName = "Character"))
	AGASPCharacter* GetCharacter() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	EGait GetGait() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	EMovementState GetMovementState() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	ECMovementMode GetMovementMode() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	EStanceMode GetStanceMode() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	ERotationMode GetRotationMode() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FCharacterInfo GetCharacterInfo() const;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetAimSweep() const;
};
