// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotifyState_EarlyTransition.h"
#include "Animation/GASPAnimInstance.h"
#include "Types/EnumTypes.h"

void UAnimNotifyState_EarlyTransition::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                  float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!IsValid(MeshComp))
	{
		return;
	}

	UGASPAnimInstance* AnimInstance = static_cast<UGASPAnimInstance*>(MeshComp->GetAnimInstance());
	if (!IsValid(AnimInstance))
	{
		return;
	}

	if (EventReference.IsActiveContext())
	{
		return;
	}

	bool bTransition = [&]()
	{
		switch (TransitionCondition)
		{
		case EEarlyTransitionCondition::GaitNotEqual:
			return AnimInstance->GetGait() != GaitNotEqual;
		default:
			return true;
		}
	}();

	if (!bTransition)
	{
		return;
	}

	switch (TransitionDestination)
	{
	case EEarlyTransitionDestination::ReTransition:
		AnimInstance->bNotifyTransition_ReTransition = true;
		break;
	default:
		AnimInstance->bNotifyTransition_ToLoop = true;
		break;
	}
}

FString UAnimNotifyState_EarlyTransition::GetNotifyName_Implementation() const
{
	FString Output{FStructEnumLibrary::GetNameStringByValue(TransitionDestination)};

	switch (TransitionCondition)
	{
	case EEarlyTransitionCondition::GaitNotEqual:
		Output.Append(TEXT(" - If - "));
		Output.Append(FStructEnumLibrary::GetNameStringByValue(TransitionCondition));
		Output.Append(TEXT(" - "));
		Output.Append(TransitionCondition == EEarlyTransitionCondition::GaitNotEqual
			              ? FStructEnumLibrary::GetNameStringByValue(GaitNotEqual)
			              : TEXT(""));
		break;
	default:
		Output.Append(TEXT(" - Always"));
		break;
	}

	return Output;
}
