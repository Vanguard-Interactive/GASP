// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotifyState_MontageBlendOut.h"
#include "Actors/GASPCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Types/EnumTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNotifyState_MontageBlendOut)

FString UAnimNotifyState_MontageBlendOut::GetNotifyName_Implementation() const
{
	FString Name{TEXT("Early Blend out - ")};
	return Name.Append(GetNameStringByValue(BlendOutCondition));
}

void UAnimNotifyState_MontageBlendOut::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                  float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!IsValid(MeshComp))
	{
		return;
	}

	const AGASPCharacter* Character = static_cast<AGASPCharacter*>(MeshComp->GetOwner());
	if (!IsValid(Character))
	{
		return;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}

	UAnimMontage* AnimMontage = static_cast<UAnimMontage*>(Animation);

	bool ShouldBlendOut = [&]()
	{
		switch (BlendOutCondition)
		{
		case ETraversalBlendOutCondition::WithMovementInput:
			return !Character->GetReplicatedAcceleration().IsNearlyZero(.1f);

		case ETraversalBlendOutCondition::IfFalling:
			return Character->GetMovementMode() == ECMovementMode::InAir;
		default:
			return true;
		}
	}();

	if (ShouldBlendOut)
	{
		FMontageBlendSettings BlendOutSettings{AnimMontage->BlendOut};
		BlendOutSettings.Blend.BlendTime = BlendOutTime;
		BlendOutSettings.Blend.BlendOption = EAlphaBlendOption::HermiteCubic;
		BlendOutSettings.Blend.CustomCurve = nullptr;
		BlendOutSettings.BlendMode = AnimMontage->BlendModeOut;
		BlendOutSettings.BlendProfile = const_cast<UBlendProfile*>(AnimInstance->
			GetBlendProfileByName(BlendProfile));

		AnimInstance->Montage_StopWithBlendSettings(BlendOutSettings, AnimMontage);
	}
}
