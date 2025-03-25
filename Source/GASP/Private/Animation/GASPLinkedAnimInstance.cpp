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
		if (!Parent.IsValid())
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
	if (Parent.IsValid())
	{
		return Parent->GetGait();
	}
	return EGait();
}

EMovementState UGASPLinkedAnimInstance::GetMovementState() const
{
	if (Parent.IsValid())
	{
		return Parent->GetMovementState();
	}
	return EMovementState();
}

FGameplayTag UGASPLinkedAnimInstance::GetMovementMode() const
{
	if (Parent.IsValid())
	{
		return Parent->GetMovementMode();
	}
	return FGameplayTag();
}

EStanceMode UGASPLinkedAnimInstance::GetStanceMode() const
{
	if (Parent.IsValid())
	{
		return Parent->GetStanceMode();
	}
	return EStanceMode();
}

ERotationMode UGASPLinkedAnimInstance::GetRotationMode() const
{
	if (Parent.IsValid())
	{
		return Parent->GetRotationMode();
	}
	return ERotationMode();
}

FCharacterInfo UGASPLinkedAnimInstance::GetCharacterInfo() const
{
	if (Parent.IsValid())
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
