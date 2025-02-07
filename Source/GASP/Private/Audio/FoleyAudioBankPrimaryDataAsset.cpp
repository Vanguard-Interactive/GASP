// Fill out your copyright notice in the Description page of Project Settings.


#include "Audio/FoleyAudioBankPrimaryDataAsset.h"
#include "GameplayTagContainer.h"

USoundBase* UFoleyAudioBankPrimaryDataAsset::GetSoundFromEvent(FGameplayTag Event)
{
	TSoftObjectPtr<USoundBase> SoundEffect = *FoleyPrimaryData.Find(Event);
	if (SoundEffect.IsValid())
	{
		return SoundEffect.LoadSynchronous();
	}
	return nullptr;
}
