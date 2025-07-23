// Fill out your copyright notice in the Description page of Project Settings.


#include "Foley/GASPFootstepEffectsSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASPFootstepEffectsSet)

FGASPFootstepEffectsSettings* UGASPFootstepEffectsSet::GetFootstepSettingsFromSurface(const EPhysicalSurface Surface)
{
	if (!IsValid(this))
	{
		return nullptr;
	}

	return FootstepSettings.Find(Surface);
}

