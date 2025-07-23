// Fill out your copyright notice in the Description page of Project Settings.


#include "Nodes/AnimNode_GameplayTagsBlend.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNode_GameplayTagsBlend)

int32 FAnimNode_GameplayTagsBlend::GetActiveChildIndex()
{
	const FGameplayTag& CurrentActiveTag{GetActiveTag()};

	return CurrentActiveTag.IsValid()
		       ? GetTags().Find(CurrentActiveTag) + 1
		       : 0;
}

const FGameplayTag& FAnimNode_GameplayTagsBlend::GetActiveTag() const
{
	return GET_ANIM_NODE_DATA(FGameplayTag, ActiveTag);
}

const TArray<FGameplayTag>& FAnimNode_GameplayTagsBlend::GetTags() const
{
	return GET_ANIM_NODE_DATA(TArray<FGameplayTag>, Tags);
}

#if WITH_EDITOR
void FAnimNode_GameplayTagsBlend::RefreshPosePins()
{
	const int Difference = BlendPose.Num() - GetTags().Num() - 1;
	if (Difference == 0)
	{
		return;
	}

	if (Difference > 0)
	{
		for (int i = Difference; i > 0; i--)
		{
			RemovePose(BlendPose.Num() - 1);
		}
	}
	else
	{
		for (int i = Difference; i < 0; i++)
		{
			AddPose();
		}
	}
}
#endif
