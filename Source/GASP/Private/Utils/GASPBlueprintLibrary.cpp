// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/GASPBlueprintLibrary.h"
#include "GameFramework/Character.h"

float UGASPBlueprintLibrary::GetAnimationCurveValueFromCharacter(const ACharacter* Character, const FName& CurveName)
{
	const auto* Mesh{IsValid(Character) ? Character->GetMesh() : nullptr};
	const auto* AnimationInstance{IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr};

	return IsValid(AnimationInstance) ? AnimationInstance->GetCurveValue(CurveName) : 0.0f;
}
