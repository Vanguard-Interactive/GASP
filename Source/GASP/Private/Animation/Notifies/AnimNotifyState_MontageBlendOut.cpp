// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotifyState_MontageBlendOut.h"

#include "Actors/GASPCharacter.h"
#include "Types/EnumTypes.h"

FString UAnimNotifyState_MontageBlendOut::GetNotifyName_Implementation() const
{
	FString Name{TEXT("Early Blend out - ")};
	return Name.Append(FStructEnumLibrary::GetNameStringByValue(BlendOutCondition));
}

void UAnimNotifyState_MontageBlendOut::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                  float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!IsValid(MeshComp))
	{
		return;
	}

	AGASPCharacter* Character = StaticCast<AGASPCharacter*>(MeshComp->GetOwner());
	if (!IsValid(Character))
	{
		return;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}

	UAnimMontage* AnimMontage = StaticCast<UAnimMontage*>(Animation);

	bool ShouldBlendOut = [&]()
	{
		switch (BlendOutCondition)
		{
		case ETraversalBlendOutCondition::WithMovementInput:
			return Character->GetReplicatedAcceleration().IsNearlyZero(.1f);

		case ETraversalBlendOutCondition::IfFalling:
			return Character->GetMovementMode() == ECMovementMode::InAir;

		default: return true;
		}
	}();

	if (!ShouldBlendOut)
	{
		return;
	}

	FMontageBlendSettings MontageBlendSettings;
	MontageBlendSettings.Blend.BlendTime = BlendOutTime;
	MontageBlendSettings.Blend.BlendOption = EAlphaBlendOption::HermiteCubic;
	MontageBlendSettings.Blend.CustomCurve = nullptr;
	MontageBlendSettings.BlendProfile = const_cast<UBlendProfile*>(AnimInstance->GetBlendProfileByName(BlendProfile));
	MontageBlendSettings.BlendMode = EMontageBlendMode::Standard;

	AnimInstance->Montage_StopWithBlendSettings(MontageBlendSettings, AnimMontage);
}
