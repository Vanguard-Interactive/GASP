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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<class AGASPCharacter> Character;

public:
	UGASPLinkedAnimInstance();

	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintPure, meta = (HideSelfPin, BlueprintThreadSafe, ReturnDisplayName = "Parent"))
	UGASPAnimInstance* GetParent() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FGait GetGait() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FMovementState GetMovementState() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FMovementMode GetMovementMode() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FStanceMode GetStanceMode() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FRotationMode GetRotationMode() const;

	UFUNCTION(BlueprintGetter, meta = (BlueprintThreadSafe))
	FCharacterInfo GetCharacterInfo() const;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetAimSweep() const;
};
