// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/MovementAnimInstanceProxy.h"

FMovementAnimInstanceProxy::FMovementAnimInstanceProxy(UAnimInstance* InAnimInstance)
	: FAnimInstanceProxy{InAnimInstance}
{
}

void FMovementAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}