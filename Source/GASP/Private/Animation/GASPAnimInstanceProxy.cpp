// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/GASPAnimInstanceProxy.h"

FGASPAnimInstanceProxy::FGASPAnimInstanceProxy(UAnimInstance* InAnimInstance)
	: FAnimInstanceProxy{InAnimInstance}
{
}

void FGASPAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);

	CharacterOwner = StaticCast<AGASPCharacter*>(InAnimInstance->TryGetPawnOwner());
}

void FGASPAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}
