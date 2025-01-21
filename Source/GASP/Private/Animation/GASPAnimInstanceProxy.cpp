// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/GASPAnimInstanceProxy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASPAnimInstanceProxy)

FGASPAnimInstanceProxy::FGASPAnimInstanceProxy(UAnimInstance* InAnimInstance)
	: FAnimInstanceProxy{InAnimInstance}
{
}

void FGASPAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);

	CharacterOwner = Cast<AGASPCharacter>(InAnimInstance->TryGetPawnOwner());
}

void FGASPAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}
