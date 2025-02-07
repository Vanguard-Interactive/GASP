// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/GASPLinkedAnimInstance.h"
#include "Animation/GASPAnimInstance.h"

UGASPLinkedAnimInstance::UGASPLinkedAnimInstance()
{
	bUseMainInstanceMontageEvaluationData = true;
}

void UGASPLinkedAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Parent = Cast<UGASPAnimInstance>(GetSkelMeshComponent()->GetAnimInstance());
	Character = Cast<AGASPCharacter>(GetOwningActor());

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

FGait UGASPLinkedAnimInstance::GetGait() const
{
	if (Parent.IsValid())
	{
		return Parent->GetGait();
	}
	return FGait();
}

FMovementState UGASPLinkedAnimInstance::GetMovementState() const
{
	if (Parent.IsValid())
	{
		return Parent->GetMovementState();
	}
	return FMovementState();
}

FMovementMode UGASPLinkedAnimInstance::GetMovementMode() const
{
	if (Parent.IsValid())
	{
		return Parent->GetMovementMode();
	}
	return FMovementMode();
}

FStanceMode UGASPLinkedAnimInstance::GetStanceMode() const
{
	if (Parent.IsValid())
	{
		return Parent->GetStanceMode();
	}
	return FStanceMode();
}

FRotationMode UGASPLinkedAnimInstance::GetRotationMode() const
{
	if (Parent.IsValid())
	{
		return Parent->GetRotationMode();
	}
	return FRotationMode();
}

FCharacterInfo UGASPLinkedAnimInstance::GetCharacterInfo() const
{
	if (Parent.IsValid())
	{
		return Parent->GetCharacterInfo();
	}
	return FCharacterInfo();
}
