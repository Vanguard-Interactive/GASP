// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstanceProxy.h"
#include "GASPAnimInstanceProxy.generated.h"

class AGASPCharacter;
/**
 * 
 */
USTRUCT()
struct FGASPAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FGASPAnimInstanceProxy() = default;

	explicit FGASPAnimInstanceProxy(UAnimInstance* InAnimInstance);

protected:
	virtual void InitializeObjects(UAnimInstance* InAnimInstance) override;
	virtual void Update(float DeltaSeconds) override;

	UPROPERTY(Transient)
	TWeakObjectPtr<AGASPCharacter> CharacterOwner;
public:
	
	FORCEINLINE AGASPCharacter* GetCharacterOwner() const { return CharacterOwner.Get(); }

};