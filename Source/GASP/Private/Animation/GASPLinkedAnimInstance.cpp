// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/GASPLinkedAnimInstance.h"
#include "Animation/GASPAnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASPLinkedAnimInstance)

UGASPLinkedAnimInstance::UGASPLinkedAnimInstance()
{
	bUseMainInstanceMontageEvaluationData = true;
}

void UGASPLinkedAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Parent = Cast<UGASPAnimInstance>(GetSkelMeshComponent()->GetAnimInstance());
	Character = Cast<AGASPCharacter>(TryGetPawnOwner());

#if WITH_EDITOR
	const auto* World{GetWorld()};

	if (IsValid(World) && !World->IsGameWorld())
	{
		// Use default objects for editor preview.
		if (!IsValid(Parent))
		{
			Parent = GetMutableDefault<UGASPAnimInstance>();
		}

		if (!IsValid(Character))
		{
			Character = GetMutableDefault<AGASPCharacter>();
		}
	}
#endif
}

UGASPAnimInstance* UGASPLinkedAnimInstance::GetParent() const
{
	return Parent.Get();
}

AGASPCharacter* UGASPLinkedAnimInstance::GetCharacter() const
{
	return Character;
}

EGait UGASPLinkedAnimInstance::GetGait() const
{
	if (IsValid(Parent))
	{
		return Parent->GetGait();
	}
	return EGait();
}

FGameplayTag
 UGASPLinkedAnimInstance::GetMovementState() const
{
	if (IsValid(Parent))
	{
		return Parent->GetMovementState();
	}
	return FGameplayTag
();
}

FGameplayTag UGASPLinkedAnimInstance::GetMovementMode() const
{
	if (IsValid(Parent))
	{
		return Parent->GetMovementMode();
	}
	return FGameplayTag();
}

FGameplayTag UGASPLinkedAnimInstance::GetStanceMode() const
{
	if (IsValid(Parent))
	{
		return Parent->GetStanceMode();
	}
	return FGameplayTag();
}

ERotationMode UGASPLinkedAnimInstance::GetRotationMode() const
{
	if (IsValid(Parent))
	{
		return Parent->GetRotationMode();
	}
	return ERotationMode();
}

FCharacterInfo UGASPLinkedAnimInstance::GetCharacterInfo() const
{
	if (IsValid(Parent))
	{
		return Parent->GetCharacterInfo();
	}
	return FCharacterInfo();
}

float UGASPLinkedAnimInstance::GetAimSweep() const
{
	return FMath::GetMappedRangeValueClamped<float, float>({-90.f, 90.f},
	                                                       {1.f, 0.f},
	                                                       GetParent()->GetAOValue().Y);
}
